#include <iostream>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "vk_core.h"
#include "vk_application.h"
#include "vk_compute.h"
using namespace std;
using namespace VKEngine;


struct {
	struct{
		size_t x,y,z;
	}size;
	float isovalue;
	float *data;
	string file_path;
	uint32_t result;
}Volume;

void loadVolume(string file_path, void *data){
	printf("load volume!\n");
	std::ifstream is(file_path, std::ios::binary | std::ios::in | std::ios::ate);
	if(is.is_open()){
		size_t size = is.tellg();
		cout << "volume size : " << size << endl;
		assert(size > 0);
		is.seekg(0, std::ios::beg);
		is.read((char *)data, size);
	}else{
		cerr << "fail to read volume data. check it first\n";
	}	
};

vector<const char *> getRequiredExtensions(  ){
	glfwInit();
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if(validationEnable) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	glfwTerminate();
	return extensions;
}

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	
	static vector<VkVertexInputBindingDescription> vertexInputBinding(){
		vector <VkVertexInputBindingDescription> bindings;
		VkVertexInputBindingDescription binding = {};
		binding.binding = 0;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = sizeof(Vertex);
		bindings.push_back(binding);
		return bindings;
	}

	static vector<VkVertexInputAttributeDescription> vertexInputAttributes(){
		vector<VkVertexInputAttributeDescription> attributes(2);
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, pos);

		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].offset = offsetof(Vertex, color);
		return attributes;
	}
};


struct RenderObject{
	struct {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	}matrices;
	Buffer *ubo = nullptr;
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	Program *program = nullptr;
};

class Scan{
	private :
	Context *ctx = nullptr;
	CommandQueue *queue = nullptr;
	VkDescriptorPool desc_pool = VK_NULL_HANDLE;
	VkPipelineCache cache = VK_NULL_HANDLE;
	public:
	Kernel scan, scan_ed, propagation;
	vector<uint32_t> g_sizes;
	vector<uint32_t> l_sizes;
	vector<uint32_t> limits;
	vector<Buffer *> d_grps;
	Buffer u_limit;

	public : 
	Scan(){}
	Scan(Context *_ctx, CommandQueue *_queue){
		create(_ctx, _queue);
	}
	~Scan(){
		destroy();
	}

	void destroy(){
		VkDevice device = VkDevice(*ctx);
		scan.destroy();
		scan_ed.destroy();
		propagation.destroy();
		vkDestroyDescriptorPool(device, desc_pool, nullptr);
		for(auto iter = d_grps.begin() ; iter != d_grps.end() ; ++iter){
			(*iter)->destroy();
		}
		u_limit.destroy();
		vkDestroyPipelineCache(device,cache,nullptr);
	}

	void create(Context *_ctx, CommandQueue *_queue){
		ctx=_ctx;
		queue=_queue;
	}

	void init(uint32_t sz_elem){
		setupDescriptorPool();
		setupKernels();
		initMem(sz_elem);
		buildKernels();
	}

	private :
	void setupDescriptorPool(){
		vector<VkDescriptorPoolSize> pool_size = {
			infos::descriptorPoolSize( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 8),
			infos::descriptorPoolSize( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
		};
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo(
			static_cast<uint32_t>(pool_size.size()),
			pool_size.data(),
			4
		);

		VK_CHECK_RESULT(vkCreateDescriptorPool(VkDevice(*ctx), &pool_CI, nullptr, &desc_pool));
	}
	void setupKernels(){
		//TODO set propagation kernel
		scan.create(ctx, "shaders/marching_cube/scan.comp.spv");
		scan_ed.create(ctx, "shaders/marching_cube/scan_ed.comp.spv");
		propagation.create(ctx, "shaders/marching_cube/uniform_update.comp.spv");
		scan.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
		});
		scan_ed.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
		});
		propagation.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1)
		});
		scan.allocateDescriptorSet(desc_pool);
		scan_ed.allocateDescriptorSet(desc_pool);
		propagation.allocateDescriptorSet(desc_pool);
	}

	void initMem(uint32_t sz_elem){
		uint32_t size = sz_elem;
		uint32_t sm = 64;
		while(size > sm*4){
			uint32_t gsiz = (size+3)/4;
			limits.push_back(gsiz);
			g_sizes.push_back((gsiz + sm - 1)/sm * sm  );
			l_sizes.push_back(sm);
			size = (gsiz + sm - 1) / sm;
			d_grps.push_back( new Buffer( ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (size+1)*4, nullptr));
		}

		if(size>0){
			d_grps.push_back(nullptr);
			g_sizes.push_back(size);
			l_sizes.push_back(size);
			limits.push_back(size);
		}
		u_limit.create(ctx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(uint32_t), &limits[0]);


		printf("g_sizes : [ ");
		for(uint32_t i = 0 ; i < g_sizes.size() ; ++i){
			printf("%d ", g_sizes[i]);
		}
		printf(" ] \n");

		printf("l_sizes : [ ");
		for(uint32_t i = 0 ; i < l_sizes.size() ; ++i){
			printf(" %d ", l_sizes[i]);
		}
		printf(" ]\n");

		printf("limits : [ ");
		for(uint32_t i : limits){
			printf(" %d ", i);
		}
		printf(" ]\n");

	}

	void buildKernels(){
		//TODO 
		//build propagion
		uint32_t s_size = limits[limits.size()-1];
		vector<uint32_t> s_data = {
			s_size * 2,
			s_size
		};
		/*
		for(uint32_t i : s_data){
			cout << "s_data : " << i << endl;
		}
		*/
		VkSpecializationMapEntry scan_ed_map[2];
		scan_ed_map[0].constantID = 0;
		scan_ed_map[0].offset = 0;
		scan_ed_map[0].size = sizeof(uint32_t);
		scan_ed_map[1].constantID = 1;
		scan_ed_map[1].offset = 0;
		scan_ed_map[1].size = sizeof(uint32_t);
		VkSpecializationInfo scan_ed_SI={};
		scan_ed_SI.mapEntryCount = 2;
		scan_ed_SI.pMapEntries = scan_ed_map;
		scan_ed_SI.dataSize = static_cast<uint32_t>(sizeof(uint32_t)*s_data.size()),
		scan_ed_SI.pData = s_data.data();
		scan.build(cache, nullptr);
		scan_ed.build(cache, &scan_ed_SI);
		propagation.build(cache, nullptr);
		cout << "scan ed local size : " << s_data[1] << endl;
	}
	public :
	void run(Buffer *d_src, Buffer *d_dst){
		// d_src = edge_test_result
		// d_dst = edge_test_psum_out
		uint32_t nr_grps, nr_g, nr_l, nr_limits;
		nr_grps = static_cast<uint32_t>( d_grps.size() );
		nr_g = static_cast<uint32_t>(g_sizes.size());
		nr_l = static_cast<uint32_t>( l_sizes.size() );
		nr_limits = static_cast<uint32_t>( limits.size() );

		vector<Buffer *> d_srcs = {d_src};
		vector<Buffer *> d_dsts = {d_dst};

		for(int i = 0 ; i < nr_grps ; ++i){
			d_srcs.push_back(d_grps[i]);
			d_dsts.push_back(d_grps[i]);
		}

		for(uint32_t i = 0 ; i < nr_grps ; ++i){
			if(d_grps[i] != nullptr){
				printf("run scan kernel\n");
				std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
				u_limit.copyFrom(&limits[i], sizeof(uint32_t));
				scan.setKernelArgs({
					{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_srcs[i]->descriptor, nullptr},
					{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dsts[i]->descriptor, nullptr},
					{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_grps[i]->descriptor, nullptr},
					{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &u_limit.descriptor, nullptr}
				});
				queue->ndRangeKernel( &scan, {g_sizes[i],1,1}, VK_FALSE);
				std::chrono::duration<double> t = std::chrono::system_clock::now() - start;
				printf("scan kernel spent : %.3f seconds\n", t.count());
			}else{
				printf("run scan_ed kernel\n");
				std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
				scan_ed.setKernelArgs({
					{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_srcs[i]->descriptor, nullptr},
					{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dsts[i]->descriptor, nullptr}
				});
				queue->ndRangeKernel( &scan_ed, {g_sizes[i],1,1}, VK_FALSE);
				std::chrono::duration<double> t = std::chrono::system_clock::now() - start;
				printf("scan_ed kernel spent : %.3f seconds\n", t.count());
			}
		}
		
		for(int i = nr_grps-1 ; i >=0 ; --i){
			if(d_grps[i] != nullptr){
				printf("uniform update : g_sizes : %d l_size : 64 \n", g_sizes[i]);
				propagation.setKernelArgs({
					{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dsts[i]->descriptor, nullptr},
					{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_grps[i]->descriptor, nullptr}
				});
				
				std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
				queue->ndRangeKernel( &propagation, {g_sizes[i],1,1}, VK_FALSE );
				std::chrono::duration<double> t = std::chrono::system_clock::now() - start;
				printf("uniform_update kernel spent : %.3f seconds\n", t.count());
			}else{
				printf("d_grps[%d] == nullptr\n", i);
			}
		}
	}
};

class MarchingCube{
	private :
	VkDescriptorPool desc_pool = VK_NULL_HANDLE;
	CommandQueue *queue = nullptr;
	Context *ctx = nullptr;
	VkPipelineCache cache = VK_NULL_HANDLE;
	public :
	struct {
		Kernel kernel;
		Buffer d_dst;
	}edge_test;

	struct {
		Kernel kernel;
	}edge_compact;

	struct {
		Kernel kernel;
		Buffer d_dst;
	}cell_test;

	struct{
		Buffer raw;
		Buffer isovalue;
	}general;

	struct{
		Buffer vertices;
		Buffer indices;
		Kernel gen_indices;
		Kernel gen_vertices;
	}output;

	struct{
		Buffer edge_out;
		Buffer cell_out;
	}prefix_sum;

	Scan edge_scan;
	Scan cell_scan;

	public :
	MarchingCube(){};
	MarchingCube(Context *_ctx, CommandQueue *_queue){
		create(_ctx, _queue);
	}

	~MarchingCube(){
		destroy();
	}

	void destroy(){
		VkDevice device = VkDevice(*ctx);
		edge_test.kernel.destroy();
		cell_test.kernel.destroy();

		edge_test.d_dst.destroy();
		cell_test.d_dst.destroy();
		general.raw.destroy();
		general.isovalue.destroy();
		output.vertices.destroy();
		output.indices.destroy();
		vkDestroyDescriptorPool(device, desc_pool, nullptr);
		vkDestroyPipelineCache(device, cache, nullptr);
	}

	void create(Context *_ctx, CommandQueue *_queue){
		ctx = _ctx;
		queue = _queue;
	}

	void init(){
		setupDescriptorPool();
		uint32_t x = Volume.size.x;
		uint32_t y = Volume.size.y;
		uint32_t z = Volume.size.z;

		edge_scan.create(ctx, queue);
		cell_scan.create(ctx, queue);
		edge_scan.init( (x-1) * (y-1) * (z-1) * 3  );
		cell_scan.init( (x-2) * (y-2) * (z-2)  );
		createKernels();
		setupBuffers();
		setupKernels();
	}

	private :
	void setupDescriptorPool(){
		vector<VkDescriptorPoolSize> pool_size = {
			infos::descriptorPoolSize( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  12),
			infos::descriptorPoolSize( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6)
		};
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo(
			static_cast<uint32_t>(pool_size.size()),
			pool_size.data(),
			5
		);

		VK_CHECK_RESULT(vkCreateDescriptorPool(VkDevice(*ctx), &pool_CI, nullptr, &desc_pool));
		cout << "MC::setupDescriptorPool : " << desc_pool << "\n";
	}

	void setupBuffers(){
		uint32_t raw_size = Volume.size.x * Volume.size.y * Volume.size.z;
		uint32_t x,y,z;;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;
		general.raw.create( ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, raw_size * sizeof(float32), nullptr);
		general.isovalue.create(ctx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(float), &Volume.isovalue);
		edge_test.d_dst.create(ctx,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * 3 * (x-1) * (y-1) * (z-1), nullptr);
		cell_test.d_dst.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uint32_t) * (x-2) * (y-2) * (z-2));

		prefix_sum.edge_out.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (x-1)*(y-1)*(z-1)*sizeof(uint32_t)*3 , nullptr);
	}

	void createKernels(){
		edge_test.kernel.create(ctx, "shaders/marching_cube/edge_test.comp.spv");
		cell_test.kernel.create(ctx, "shaders/marching_cube/cell_test.comp.spv");
		edge_compact.kernel.create(ctx, "shaders/marching_cube/edge_compact.comp.spv");
		//output.gen_vertices.create(ctx, "shaders/marching_cube/gen_vertices.comp.spv");
		//output.gen_indices.create(ctx, "shaders/marching_cube/gen_indices.comp.spv");
	}

	void setupKernels(){
		edge_test.kernel.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0 ),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1 ),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2)
		});

		edge_compact.kernel.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2)
		});

		cell_test.kernel.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3)
		});

		edge_test.kernel.allocateDescriptorSet(desc_pool);
		edge_compact.kernel.allocateDescriptorSet(desc_pool);
		cell_test.kernel.allocateDescriptorSet(desc_pool);
		edge_test.kernel.build(cache, nullptr);
		edge_compact.kernel.build(cache, nullptr);
		cell_test.kernel.build(cache, nullptr);
	}

	public : 
	void setupVolume(){
		//void *data = Volume.data;
		uint32_t sz_volume = Volume.size.x * Volume.size.y * Volume.size.z;
		queue->enqueueCopy(Volume.data, &general.raw, 0, 0, sizeof(float) * sz_volume);
	}
	void cellTest(){
		uint32_t x = Volume.size.x-2;
		uint32_t y = Volume.size.y-2;
		uint32_t z = Volume.size.z-2;

		cell_test.kernel.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.raw.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &cell_test.d_dst.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.isovalue.descriptor, nullptr}
		});

		queue->ndRangeKernel(&cell_test.kernel, {x,y,z}, VK_FALSE);
	}

	void edgeTest(){
		uint32_t gx = Volume.size.x-1;
		uint32_t gy = Volume.size.y-1;
		uint32_t gz = Volume.size.z-1;
		edge_test.kernel.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.raw.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &edge_test.d_dst.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &general.isovalue.descriptor, nullptr}
		});
		queue->ndRangeKernel(&edge_test.kernel, {gx,gy,gz}, VK_FALSE);
		//uint32_t *out_et = new uint32_t[3*gx * gy * gz];
		//queue->enqueueCopy(&edge_test.d_dst, out_et, 0, 0, sizeof(uint32_t) * 3 * gx * gy * gz);
		//uint32_t sum = 0;
		//for(uint32_t i = 0 ; i < 3*gx*gy*gz ; ++i)
		//	sum+=out_et[i];
		//printf("edge_test result : %d\n", sum);
		//delete [] out_et;
	}

	void edgeTestPrefixSum(){
		uint32_t x,y,z;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;
		//printf("edge_test_prefix_sum start!\n");
		edge_scan.run( &edge_test.d_dst, &prefix_sum.edge_out);
		//uint32_t psum_out = 0;
		//printf("edge_psum_out : %d\n", psum_out);
		//printf("edge_test_prefix_sum done!\n");
	}

	void cellTestPrefixSum(){
		uint32_t x,y,z;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;
		cell_scan.run( &cell_test.d_dst, &prefix_sum.cell_out);
	}

	void edgeCompact(){
		printf("edge compact start\n");
		uint32_t psum_out = 0;
		uint32_t x,y,z;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;
		queue->enqueueCopy(&prefix_sum.edge_out, &psum_out, (3*(x-1)*(y-1)*(z-1) - 1  ) * sizeof(uint32_t) , 0,  sizeof(uint32_t));

		printf("target allocated size : %d\n",  psum_out);
		output.vertices.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, psum_out * sizeof(float) * 3,  nullptr);
		printf("actual allocated size : %d\n", psum_out);
		edge_compact.kernel.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &output.vertices.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &edge_test.d_dst.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &prefix_sum.edge_out.descriptor, nullptr}
		});

		printf("set kernel asgrs done\n");
		queue->ndRangeKernel(&edge_compact.kernel, {3*(x-1)*(y-1)*(z-1), 1, 1}, VK_FALSE);
		uint32_t *ec_out  = new uint32_t[  3*psum_out ];
		queue->enqueueCopy(&output.vertices, ec_out, 0, 0, sizeof(uint32_t) * 3 * (psum_out));
		uint32_t sum = 0;
		for(uint32_t i = 0 ; i < 10 ; ++i){
			printf("%d ", ec_out[i*3]);	
		}
		printf("\n");
		for(uint32_t i = psum_out - 1 ; i > psum_out - 11 ; i--){
			printf("%d ", ec_out[i*3]);
		}
		printf("\n");
		delete [] ec_out;
		printf("edge_compact result : %d\n", sum);
		printf("edge_compact done\n");
	}
	void generateVertices(){

	}
	void generateIndices(){
	}
};


class App : public VKEngine::Application{
	public :

	explicit App(string app_name, string engine_name, int h, int w, vector<const char*>instance_exts, vector<const char*>device_exts , vector<const char *>valids) : Application(app_name, engine_name, h, w, instance_exts, device_exts, valids){
	};

	vector<RenderObject> render_objects;
	
	MarchingCube mc;

	public :
	~App(){
		VkDevice device = VkDevice(*context);
		VkCommandPool command_pool = VkCommandPool(*compute_queue);
		mc.destroy();
		//vkFreeCommandBuffers(device, command_pool, 1, &compute.command_buffer);
	}

	protected:
	virtual void initWindow(){
		LOG("App Init Window\n");
		glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
	}

	virtual void createSurface(){
		LOG("createSurface()\n");
		VK_CHECK_RESULT(glfwCreateWindowSurface(VkInstance(*engine), window, nullptr, &surface));
	}

	virtual void mainLoop(){
		while(!glfwWindowShouldClose(window)){
			glfwPollEvents();
			draw();
		}
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void draw(){
		render();
	}

	void prepareComputeBuffers(){
		/*
		size_t volume_size = Volume.size.x * Volume.size.y * Volume.size.z;
		volume.host.raw.create(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
						 	   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						  	   volume_size * sizeof(float), Volume.data);
		volume.device.raw.create(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, volume_size * sizeof(float), nullptr);
		volume.device.v_test.create(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, volume_size * sizeof(uint32_t), nullptr);
		volume.host.iso_value.create(context, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									sizeof(float), &Volume.isovalue);
		volume.host.iso_value.map(0, 4);
		compute_queue->enqueueCopy( &volume.host.raw, &volume.device.raw, 0, 0, volume_size*sizeof(float) );
		*/
	}

	void prepareComputeKernels(){
		/*
		VkDevice device = VkDevice(*context);
		vector<VkDescriptorPoolSize> pool_sizes({
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2),
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
		});
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo( static_cast<uint32_t>(pool_sizes.size()), pool_sizes.data(), 1);
		VK_CHECK_RESULT(vkCreateDescriptorPool( device,  &pool_CI, nullptr, &compute.pool));
		compute.volume_test.create(context, "shaders/marching_cube/volume_test.comp.spv");
		compute.volume_test.setupDescriptorSetLayout(
			{
				infos::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , VK_SHADER_STAGE_COMPUTE_BIT,  0),
				infos::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER , VK_SHADER_STAGE_COMPUTE_BIT,  1),
				infos::descriptorSetLayoutBinding( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2)
			}
		);
		compute.volume_test.allocateDescriptorSet( compute.pool );
		cout << "kernel build start\n";
		compute.volume_test.build(cache,nullptr);
		cout << "kernel build done\n";
		*/
		/*
		compute.volume_test.setKernelArgs( { 
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &volume.device.raw.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &volume.device.v_test.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &volume.host.iso_value.descriptor, nullptr}
		});
		*/
		cout << "prepare compute kernel complete\n";
	}

	void buildComputeCommandBuffers(){
		/*
		compute.command_buffer = compute_queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		compute_queue->beginCommandBuffer(compute.command_buffer);
		//compute_queue->ndRangeKernel(&compute.volume_test, {256,256,256}, VK_TRUE);

		vkCmdBindPipeline(compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.volume_test.pipeline);
		vkCmdBindDescriptorSets(compute.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
								compute.volume_test.layout, 
								0, 1, &compute.volume_test.descriptors.set, 0, nullptr);
		vkCmdDispatch(compute.command_buffer, Volume.size.x, Volume.size.y, Volume.size.z);
		compute_queue->endCommandBuffer(compute.command_buffer);
		*/
	}
	
	void preparePrograms(){
		// LOG("-------------Test::preparePrograms() start------------------------\n");
		// Program *program = new Program(context);
		// program->attachShader("./shaders/cubes/cube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		// program->attachShader("./shaders/cubes/cube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		// program->setupDescriptorSetLayout({
		// 	infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1)
		// });
		// program->createDescriptorPool({
		// 	infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
		// });
		// auto attributes = Vertex::vertexInputAttributes();
		// auto bindings = Vertex::vertexInputBinding();
		// program->graphics.vertex_input = infos::vertexInputStateCreateInfo(attributes, bindings);
		// program->build(render_pass, cache);
		// programs.insert({"cube", program});
		// LOG("-------------Test::preparePrograms() end------------------------\n");
	}

	void prepareRenderObjects(){
	}

	void prepareCommandBuffer(){
		// std::array<VkClearValue, 2> clear_values{};
		// clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		// clear_values[1].depthStencil = {1.0f, 0};
		// draw_command_buffers.resize(swapchain.buffers.size());
		
		// VkRenderPassBeginInfo render_pass_BI = infos::renderPassBeginInfo();
		// render_pass_BI.clearValueCount = static_cast<uint32_t>(clear_values.size());
		// render_pass_BI.pClearValues = clear_values.data();
		// render_pass_BI.renderArea.offset = {0,0};
		// render_pass_BI.renderArea.extent.height = height;
		// render_pass_BI.renderArea.extent.width = width;
		// render_pass_BI.renderPass = render_pass;

		// for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
		// 	draw_command_buffers[i] = graphics_queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		// }
		
		// for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			
			// graphics_queue->beginCommandBuffer(draw_command_buffers[i]);
			// render_pass_BI.framebuffer = framebuffers[i];
			// VkViewport viewport = infos::viewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
			// VkRect2D scissor = infos::rect2D(width, height, 0, 0);
			// vkCmdBeginRenderPass(draw_command_buffers[i], &render_pass_BI, VK_SUBPASS_CONTENTS_INLINE);
			// vkCmdSetViewport(draw_command_buffers[i], 0, 1, &viewport);
			// vkCmdSetScissor(draw_command_buffers[i], 0, 1, &scissor);
			// vkCmdBindPipeline(draw_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, render_objects[0].program->pipeline);
			// VkBuffer vertex_buffer[] = {VkBuffer(*cube.vbo)};
			// VkBuffer indices_buffer[] = {VkBuffer(*cube.ibo)};
			// VkDeviceSize offsets[] = {0};
			// vkCmdBindVertexBuffers(draw_command_buffers[i], 0, 1, vertex_buffer, offsets);
			// vkCmdBindIndexBuffer(draw_command_buffers[i], VkBuffer(*cube.ibo), 0, VK_INDEX_TYPE_UINT16);
			// vkCmdBindDescriptorSets(draw_command_buffers[i], 
			// 						VK_PIPELINE_BIND_POINT_GRAPHICS,
			// 						render_objects[0].program->pipeline_layout, 
			// 						0, 
			// 						1, &render_objects[0].descriptor_set,
			// 						0, nullptr);
			// vkCmdDrawIndexed(draw_command_buffers[i], static_cast<uint32_t>(cube_indices.size()), 1, 0, 0, 0);
			// vkCmdEndRenderPass(draw_command_buffers[i]);
			// graphics_queue->endCommandBuffer(draw_command_buffers[i]);
		// }	
	}

	void executeCompute(){
		/*
		LOG("execute Compute\n");
		memcpy(volume.host.iso_value.data, &Volume.isovalue, sizeof(float));
		VkSubmitInfo submit_info = infos::submitInfo();
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &compute.command_buffer;
		vkQueueSubmit(VkQueue(*compute_queue), 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(VkQueue(*compute_queue));
		LOG("execute Compute done\n");
		*/
	}

	void copyResult(){
		/*
		size_t sz_mem = Volume.size.x * Volume.size.y * Volume.size.z * sizeof(bool);
		Buffer h_volume_test(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
			,VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sz_mem * sizeof(uint32_t), nullptr);
		compute_queue->enqueueCopy(&volume.device.v_test, &h_volume_test, 0, 0, sz_mem*sizeof(uint32_t));

		uint32_t nr_iter = Volume.size.x * Volume.size.y * Volume.size.z;
		uint32_t *vtest = new uint32_t[sz_mem];
		h_volume_test.copyTo( vtest, sz_mem  * sizeof(uint32_t));
		uint32_t vtest_sum = 0;

		for(uint32_t i = 0 ; i < nr_iter ; ++i){
			vtest_sum += (uint32_t)vtest[i];
		}

		printf("target_iso_value : %f\n vtest sum : %d\n", Volume.isovalue, vtest_sum);
		printf("CPU result : %d\n", Volume.result);

		delete[] vtest;
		*/
	}


	public:

	void runMarchingCube(){
		mc.create(context, compute_queue);
		mc.init();
		mc.setupVolume();
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		mc.edgeTest();
		std::chrono::duration<double> t = std::chrono::system_clock::now() - start;
		printf("edge_test operation time : %.4f seconds\n", t.count() );
		start = std::chrono::system_clock::now();
		mc.edgeTestPrefixSum();
		t = std::chrono::system_clock::now() - start;
		printf("edge_scan operation time : %.4f seconds\n", t.count() );

		start = std::chrono::system_clock::now();
		mc.edgeCompact();
		t = std::chrono::system_clock::now() - start;
		printf("edge_compact operation time : %.4f seconds\n", t.count() );
	}

	void prefixSumTest(){
		VkDevice device = VkDevice(*context);
		uint32_t src[512];
		uint32_t dst[512];
		uint32_t limit = 128;
		memset(dst, 0x00, sizeof(uint32_t)*128);
		for(uint32_t i = 0 ; i < 512 ; ++i){
			src[i] = 1;
		}

		Buffer h_src(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
							  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 512*4, (void *)src);
		Buffer d_src(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
					 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 512*4, nullptr);
		Buffer d_dst(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT , 
					 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 512*4, nullptr);
		Buffer d_limit(context, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 4, nullptr);

		compute_queue->enqueueCopy(&src, &d_src, 0, 0, 512 * 4);
		compute_queue->enqueueCopy(&dst, &d_dst, 0, 0, 512 * 4);
		compute_queue->enqueueCopy(&limit, &d_limit, 0, 0, 4);
		Kernel scan(context, "shaders/marching_cube/scan.comp.spv");
		Kernel scan_ed(context, "shaders/marching_cube/scan_ed.comp.spv");

		VkDescriptorPool pool;
		vector<VkDescriptorPoolSize> pool_sizes = {
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,6),
			infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
		};
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo(static_cast<uint32_t>(pool_sizes.size()), pool_sizes.data(), 1);
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_CI, nullptr, &pool));
		
		scan.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2)
		});

		scan_ed.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1)
		});


		scan.allocateDescriptorSet(pool);
		scan.build(cache, nullptr);



		printf("build done\n");
		scan.setKernelArgs({{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_src.descriptor,  nullptr},
							{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dst.descriptor,  nullptr},
							//{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dst.descriptor,  nullptr},
							{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &d_limit.descriptor, nullptr}});

		printf("ndRangeKernel ready\n");
		compute_queue->ndRangeKernel(&scan,{128,1,1}, VK_TRUE);
		compute_queue->enqueueCopy(&d_dst, &dst, 0, 0, 512 * 4);
		printf("[");
		for(int i = 0 ; i < 512 ; ++i){
			printf("%d, ", dst[i]);
		}
		printf("]\n");
	}

	void run(){
		Application::init();
		cout << "compute_queue : " << compute_queue << endl;
		runMarchingCube();
		//prefixSumTest();
		//prepareComputeBuffers();
		//prepareComputeKernels();
		//buildComputeCommandBuffers();
		//executeCompute();
		//copyResult();
		
		
		
		// preparePrograms();
		// prepareRenderObjects();
		// prepareCommandBuffer();
		// mainLoop();
	}
};

int main(int argc, const char *argv[])	
{
	vector<const char*> instance_extensions(getRequiredExtensions());
	vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
	vector<const char *>device_extensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	string _name = "vulkan";
	string engine_name = "engine";
	
	/*
	string file_path(argv[1]);
	size_t x = atoi(argv[2]);
	size_t y = atoi(argv[3]);
	size_t z = atoi(argv[4]);
	
	float isovalue = atof(argv[5]);
	*/
	string file_path = "assets/dragon_vrip_FLT32_128_128_64.raw";
	Volume.file_path = file_path;
	cout << "Volume file path set \n";
	Volume.size = {128,128,64};
	cout << "Volume size set \n";
	Volume.isovalue = 0.02;
	cout << "volume isovalue set done\n";

	Volume.data = new float[128*128*64];
	loadVolume(file_path, Volume.data);
	
	size_t nr_workitems = 128*128*64;
	uint32_t sum = 0 ;
	for(uint32_t i = 0 ; i < nr_workitems ; ++i){
		sum = (Volume.data[i] > Volume.isovalue) ? sum + 1 : sum;
	}
	
	Volume.result = sum;


	try {
	    App app(_name, engine_name, 600, 800, instance_extensions, device_extensions , validations);
	    app.run();
	}catch(std::runtime_error& e){
		cout << "error occured : " << e.what()  <<  "on File " << __FILE__ << " line : " << __LINE__ << "\n";
		exit(EXIT_FAILURE);
	};


	delete [] Volume.data;
	return 0;
}
