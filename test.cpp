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
#include <algorithm>
#include <ctime>
#include <unistd.h>
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN 1
#endif

#include "vk_core.h"
#include "vk_application.h"
#include "vk_compute.h"
using namespace std;
using namespace VKEngine;


#define PROFILING(FPTR, FNAME) ({ \
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now(); \
		FPTR; \
		std::chrono::duration<double> t = std::chrono::system_clock::now() - start; \
		printf("%s operation time : %.4lf seconds\n",FNAME, t.count()); \
})

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
		vector<VkVertexInputAttributeDescription> attributes(1);
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, pos);
		return attributes;
	}
};

struct UniformMatrices{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	VkDescriptorSet desc_set;
}uniform_matrix;

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
	
	VkCommandBuffer cmd_buffer;

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
			g_sizes.push_back((gsiz + sm - 1)/sm*sm  );
			l_sizes.push_back(sm);
			size = (gsiz + sm - 1) / sm;
			d_grps.push_back( new Buffer( ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
										 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (size+1)*sizeof(uint32_t), nullptr));
		}

		if(size){
			d_grps.push_back(nullptr);
			g_sizes.push_back(size);
			l_sizes.push_back(size);
			limits.push_back(size);
		}
		u_limit.create(ctx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(uint32_t), &limits[0]);
		}

	void buildKernels(){
		//TODO 
		//build propagion
		uint32_t s_size = limits[limits.size()-1];
		vector<uint32_t> s_data = {
			s_size*2,
			s_size
		};
		
		for(uint32_t i : s_data){
			cout << "s_data : " << i << endl;
		}
		
		VkSpecializationMapEntry scan_ed_map[2];
		scan_ed_map[0].constantID = 1;
		scan_ed_map[0].offset = 0;
		scan_ed_map[0].size = sizeof(uint32_t);
		scan_ed_map[1].constantID = 2;
		scan_ed_map[1].offset = 4;
		scan_ed_map[1].size = sizeof(uint32_t);
		VkSpecializationInfo scan_ed_SI={};
		scan_ed_SI.mapEntryCount = 2;
		scan_ed_SI.pMapEntries = scan_ed_map;
		scan_ed_SI.dataSize = static_cast<uint32_t>(sizeof(uint32_t)*s_data.size()),
		scan_ed_SI.pData = s_data.data();
		scan.build(cache, nullptr);
		scan_ed.build(cache, &scan_ed_SI);
		propagation.build(cache, nullptr);
		//cout << "scan ed local size : " << s_data[1] << endl;
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

		VkCommandBuffer scan_command = queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 
										0, false);
		VkFence fence = queue->createFence(VK_FENCE_CREATE_SIGNALED_BIT);
		for(uint32_t i = 0 ; i < nr_grps ; ++i){
			if(d_grps[i] != nullptr){
				//printf("run scan kernel\n");
				//std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
				u_limit.copyFrom(&limits[i], sizeof(uint32_t));
				scan.setKernelArgs({
					{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_srcs[i]->descriptor, nullptr},
					{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dsts[i]->descriptor, nullptr},
					{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_grps[i]->descriptor, nullptr},
					{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &u_limit.descriptor, nullptr}
				});
				queue->beginCommandBuffer(scan_command, 0);
				if(i == 0 ){
					d_srcs[i]->barrier(scan_command, 
										VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
										VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				}else{
					d_srcs[i]->barrier(scan_command, 
										VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
										VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
					d_grps[i]->barrier(scan_command,
								VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
								VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				}
				queue->bindKernel(scan_command, &scan);
				queue->dispatch(scan_command, g_sizes[i], 1, 1);
				queue->endCommandBuffer(scan_command);
				queue->resetFences(&fence, 1);
				queue->submit(&scan_command, 1, 0, nullptr, 0, nullptr, 0 , fence);
				queue->waitFences(&fence, 1);
			}else{
				queue->beginCommandBuffer(scan_command);
				scan_ed.setKernelArgs({
					{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_srcs[i]->descriptor, nullptr},
					{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dsts[i]->descriptor, nullptr},
				});

				d_srcs[i]->barrier(scan_command, 
									VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
									VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				d_dsts[i]->barrier(scan_command,
									VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
									VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
				);
				queue->bindKernel(scan_command, &scan_ed);
				queue->dispatch(scan_command, g_sizes[i], 1, 1);
				queue->endCommandBuffer(scan_command);
				queue->resetFences(&fence, 1);
				queue->submit(&scan_command, 1, 0, nullptr, 0, nullptr, 0 , fence);
				queue->waitFences(&fence, 1);
			}
		}
		
		for(int i = nr_grps-1 ; i >=0 ; --i){
			if(d_grps[i] != nullptr){
				//printf("run uniform_update() \n");
				queue->beginCommandBuffer(scan_command);
				propagation.setKernelArgs({
					{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_dsts[i]->descriptor, nullptr},
					{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &d_grps[i]->descriptor, nullptr}
				});
				
				d_dsts[i]->barrier(scan_command, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
									VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
				queue->bindKernel(scan_command, &propagation);
				queue->dispatch(scan_command, g_sizes[i], 1, 1);
				queue->endCommandBuffer(scan_command);
				queue->resetFences(&fence, 1);
				queue->submit(&scan_command, 1, 0, nullptr, 0, nullptr, 0 , fence);
				queue->waitFences(&fence, 1);
   			}
		}
		
		queue->waitIdle();
		queue->free(scan_command);
		queue->destroyFence(fence);
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
		Buffer cell_types;
		Buffer tri_counts;
	}cell_test;

	struct{
		Buffer raw;
		Buffer isovalue;
		Buffer dim;
		Buffer cast_table;
	}general;

	struct{
		Buffer vertices;
		Buffer indices;
		Buffer ubo;
		Kernel gen_indices;
		Kernel gen_vertices;
		uint32_t nr_faces = 0;
		uint32_t nr_vertices = 0;
		uint32_t nr_normals = 0;
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

	void save(){
		printf("save() start\n");
		uint32_t x,y,z;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;

		float *vertices = new float[3*output.nr_vertices];
		uint32_t *indices = new uint32_t[3*output.nr_faces];
		queue->enqueueCopy(&output.vertices, vertices, 0, 0 ,3 * output.nr_vertices*sizeof(float) );
		queue->enqueueCopy(&output.indices, indices, 0, 0, 3 * output.nr_faces*sizeof(uint32_t));

		std::ofstream os("sample.obj");
		for(uint32_t i = 0 ; i < output.nr_vertices ; ++i){
			os << "v " << vertices[3*i] << " " << vertices[3*i + 1] << " " << vertices[3*i + 2] << endl;
		}
			
		for(uint32_t i = 0 ; i < output.nr_faces ; ++i){
			os << "f " << indices[3*i] + 1 << " " << indices[3*i + 1]  + 1<< " " << indices[3*i + 2] + 1<< endl;
		}

		delete [] vertices;
		delete [] indices;
		os.close();
		printf("save end()\n");
	}

	void destroy(){
		VkDevice device = VkDevice(*ctx);
		edge_test.kernel.destroy();
		edge_scan.destroy();
		cell_test.kernel.destroy();
		edge_test.d_dst.destroy();

		cell_scan.destroy();
		cell_test.cell_types.destroy();
		cell_test.tri_counts.destroy();

		prefix_sum.cell_out.destroy();
		prefix_sum.edge_out.destroy();
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
		printf("MarchingcCube::setupDescriptorPool() start\n");
		vector<VkDescriptorPoolSize> pool_size = {
			infos::descriptorPoolSize( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  30),
			infos::descriptorPoolSize( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
		};
		VkDescriptorPoolCreateInfo pool_CI = infos::descriptorPoolCreateInfo(
			static_cast<uint32_t>(pool_size.size()),
			pool_size.data(),
			7
		);

		VK_CHECK_RESULT(vkCreateDescriptorPool(VkDevice(*ctx), &pool_CI, nullptr, &desc_pool));
		printf("MarchingcCube::setupDescriptorPool() end()\n");
	}

	void setupBuffers(){
		printf("MarchingCube::setupBuffers start()\n");
		uint32_t raw_size = Volume.size.x * Volume.size.y * Volume.size.z;
		uint32_t x,y,z;;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;
		general.raw.create( ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, raw_size * sizeof(float32), nullptr);

		general.isovalue.create(ctx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(float), &Volume.isovalue);

		edge_test.d_dst.create(ctx,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * 3 * (x-1) * (y-1) * (z-1), nullptr);

		cell_test.tri_counts.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uint32_t) * (x-2) * (y-2) * (z-2));

		cell_test.cell_types.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uint32_t) * (x-2) * (y-2) * (z-2));

		prefix_sum.edge_out.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 3*(x-1)*(y-1)*(z-1)*sizeof(uint32_t), nullptr);

		prefix_sum.cell_out.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, (x-2)*(y-2)*(z-2)*sizeof(uint32_t) , nullptr);

		general.dim.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(float)*3, nullptr);

		general.cast_table.create(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t)*12, nullptr);

		output.vertices.create(ctx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(float) * ((3 * (x-1) * (y-1) * (z-1)) / 4), nullptr);
		output.indices.create(ctx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * ((3 * (x-1) * (y-1) * (z-1)) / 4), nullptr);

		output.ubo.create(ctx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
							sizeof(UniformMatrices), &uniform_matrix);

		uint32_t dim[3] = {x,y,z};
		uint32_t edim[3] = {x-1,y-1,z-1};
		uint32_t cast_table[12] = {
			0, 
			4, 
			edim[0] * 3 , 
			1,
			3*edim[0]*edim[1],
			3*edim[0]*edim[1]+4,
			3*(edim[0]*edim[1]+edim[0]),
			3*edim[0]*edim[1]+1,
			2,
			5,
			3*(edim[0]+1) + 2,
			3*edim[0] + 2
		};
		queue->enqueueCopy(dim, &general.dim, 0, 0, sizeof(uint32_t)*3);
		queue->enqueueCopy(cast_table, &general.cast_table, 0, 0, sizeof(uint32_t)*12);
		printf("MarchingCube::setupBuffers end()\n");
	}

	void createKernels(){
		printf("MarchingCube::createKernel() start\n");
		edge_test.kernel.create(ctx, "shaders/marching_cube/edge_test.comp.spv");
		cell_test.kernel.create(ctx, "shaders/marching_cube/cell_test.comp.spv");
		edge_compact.kernel.create(ctx, "shaders/marching_cube/edge_compact.comp.spv");
		output.gen_vertices.create(ctx, "shaders/marching_cube/gen_vertices.comp.spv");
		output.gen_indices.create(ctx, "shaders/marching_cube/gen_indices.comp.spv");
		printf("MarchingCube::createKernel() end\n");
	}

	void setupKernels(){
		printf("MarchingCube::setupKernel() start\n");
		edge_test.kernel.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
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
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 4)
		});
		
		output.gen_vertices.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 4),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 5)
		});

		output.gen_indices.setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 4),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 5),
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 6),
		});

		edge_test.kernel.allocateDescriptorSet(desc_pool);
		edge_compact.kernel.allocateDescriptorSet(desc_pool);
		cell_test.kernel.allocateDescriptorSet(desc_pool);
		output.gen_vertices.allocateDescriptorSet(desc_pool);
		output.gen_indices.allocateDescriptorSet(desc_pool);


		edge_test.kernel.build(cache, nullptr);
		edge_compact.kernel.build(cache, nullptr);
		cell_test.kernel.build(cache, nullptr);
		output.gen_indices.build(cache,nullptr);
		output.gen_vertices.build(cache, nullptr);
		printf("MarchingCube::setupKernel() end()\n");
	}

	public : 
	void setupVolume(){
		//void *data = Volume.data;
		uint32_t sz_volume = Volume.size.x * Volume.size.y * Volume.size.z;
		queue->enqueueCopy(Volume.data, &general.raw, 0, 0, sizeof(float) * sz_volume);
	}
	void cellTest(){
		printf("cellTest() start\n");
		uint32_t x = Volume.size.x-2;
		uint32_t y = Volume.size.y-2;
		uint32_t z = Volume.size.z-2;

		cell_test.kernel.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.raw.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &cell_test.tri_counts.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &cell_test.cell_types.descriptor, nullptr},
			{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &general.isovalue.descriptor, nullptr},
			{4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.dim.descriptor, nullptr},
		});

		queue->ndRangeKernel(&cell_test.kernel, {x, y, z});
		printf("cellTest() end\n");
	}

	void edgeTest(){
		printf("edgeTest() start\n");
		uint32_t gx = Volume.size.x-1;
		uint32_t gy = Volume.size.y-1;
		uint32_t gz = Volume.size.z-1;
		edge_test.kernel.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.raw.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &edge_test.d_dst.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &general.isovalue.descriptor, nullptr}
		});
		//queue->ndRangeKernel(&edge_test.kernel, {gx,gy,gz});
		
		VkFence fence = queue->createFence();
		VkCommandBuffer command = queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, true);
		queue->bindKernel(command, &edge_test.kernel);
		queue->dispatch(command, gx , gy, gz);
		queue->endCommandBuffer(command);
		queue->resetFences(&fence, 1);
		queue->submit(&command, 1, 0, nullptr, 0, nullptr, 0, fence);
		queue->waitFences(&fence, 1);
		queue->waitIdle();
		queue->destroyFence(fence);
		queue->free(command);
		printf("edgeTest() end()\n");
	}

	void edgeTestPrefixSum(){
		printf("edgeScan() start\n");
        edge_scan.run( &edge_test.d_dst, &prefix_sum.edge_out);
        printf("edgeScan() end\n");
	}

	void cellTestPrefixSum(){
		printf("cellScan() start\n");
        cell_scan.run(&cell_test.tri_counts, &prefix_sum.cell_out);
		printf("cellScan() end\n");
	}

	void edgeCompact(){
		printf("edgeCompact() start\n");
		uint32_t x,y,z;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;
		
		/*
		queue->enqueueCopy(&prefix_sum.edge_out,
							&output.nr_vertices,
							sizeof(uint32_t)*(x-1)*(y-1)*(z-1)*3 - sizeof(uint32_t) , 0, 
							sizeof(uint32_t));
		*/
		edge_compact.kernel.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &output.vertices.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &edge_test.d_dst.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &prefix_sum.edge_out.descriptor, nullptr}
		});

		Buffer staging(ctx, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
						sizeof(uint32_t), nullptr);

		
		VkFence fence = queue->createFence();	
		VkCommandBuffer copy_command = queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, true);
		VkCommandBuffer command = queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, true);
		//prefix_sum.edge_out.barrier(command, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		queue->copyBuffer(copy_command, &prefix_sum.edge_out, &staging, sizeof(uint32_t)*(x-1)*(y-1)*(z-1) * 3 - sizeof(uint32_t), 0, sizeof(uint32_t));
		staging.barrier(copy_command, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
		queue->endCommandBuffer(copy_command);
		queue->resetFences(&fence, 1);
		queue->submit(&copy_command, 1, 0, nullptr, 0, nullptr, 0, fence);
		queue->waitFences(&fence, 1, true, UINT32_MAX);
		queue->free(copy_command);
		staging.copyTo(&output.nr_vertices, sizeof(uint32_t));
		staging.destroy();

		printf("edgeCompact()::nr_vertices : %d\n",output.nr_vertices);
		//queue->ndRangeKernel(&edge_compact.kernel, {3*(x-1)*(y-1)*(z-1), 1, 1});
		
		queue->bindKernel(command, &edge_compact.kernel);
		queue->dispatch(command, 3*(x-1)*(y-1)*(z-1), 1, 1);
		queue->endCommandBuffer(command);
		queue->resetFences(&fence, 1);
		queue->submit(&command, 1, 0, nullptr, 0, nullptr, 0, fence);
		queue->waitFences(&fence, 1, true, UINT64_MAX);
		queue->waitIdle();
		queue->free(command);
		queue->destroyFence(fence);

		printf("edgeCompact() end\n");
	}

	void generateVertices(){
		printf("genVertices() start\n");
		uint32_t x,y,z;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;
		uint32_t dim[3] = {x,y,z};
		queue->enqueueCopy(dim, &general.dim,0,0,sizeof(uint32_t)*3);

		output.gen_vertices.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.raw.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &prefix_sum.edge_out.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &output.vertices.descriptor, nullptr},
			{3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.dim.descriptor, nullptr},
			{4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &general.isovalue.descriptor, nullptr},
			{5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &output.vertices.descriptor, nullptr}
		});

		queue->ndRangeKernel(&output.gen_vertices, {output.nr_vertices, 1, 1});
        queue->waitIdle();
		printf("genVertices() end\n");
	}
	void generateIndices(){
		printf("genIndices() start\n");
		uint32_t x,y,z;
		x = Volume.size.x;
		y = Volume.size.y;
		z = Volume.size.z;

        queue->waitIdle();
		queue->enqueueCopy(&prefix_sum.cell_out, &output.nr_faces, sizeof(uint32_t) *(x-2)*(y-2)*(z-2) - sizeof(uint32_t), 0,sizeof(uint32_t));
        queue->waitIdle();

		printf("generateIndices() : nr_faces %d\n", output.nr_faces);
		output.gen_indices.setKernelArgs({
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &cell_test.cell_types.descriptor, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &cell_test.tri_counts.descriptor, nullptr},
			{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &prefix_sum.cell_out.descriptor, nullptr},
			{3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &prefix_sum.edge_out.descriptor, nullptr},
			{4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.dim.descriptor, nullptr},
			{5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &output.indices.descriptor, nullptr},
			{6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &general.cast_table.descriptor, nullptr}
		});

		queue->ndRangeKernel(&output.gen_indices, {x-2, y-2, z-2});
        queue->waitIdle();
		printf("genVertices() end\n");
	}
};


class App : public VKEngine::Application{
	public :
	MarchingCube mc;
	explicit App(string app_name, string engine_name, int h, int w, vector<const char*>instance_exts, vector<const char*>device_exts , vector<const char *>valids) : Application(app_name, engine_name, h, w, instance_exts, device_exts, valids){
	};

	~App(){
		VkDevice device = VkDevice(*context);
		VkCommandPool command_pool = VkCommandPool(*compute_queue);
		mc.destroy();
	}

	protected:
	Program *draw_program;
	
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
			updateUniform();
		}
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void draw(){
		render();
	}


	void prepareCompute(){
		uniform_matrix.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		uniform_matrix.model = glm::rotate(uniform_matrix.model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		uniform_matrix.view = glm::lookAt(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));//camera.matrices.view;
		uniform_matrix.proj = glm::perspective(glm::radians(45.0f), (width/(float)height), 0.1f, 10.0f);//camera.matrices.proj;
		uniform_matrix.proj[1][1] *= -1;
		mc.create(context, compute_queue);
		mc.init();
		mc.setupVolume();
	}
	
	void preparePrograms(){
		draw_program = new Program(context);
		draw_program->attachShader("./shaders/marching_cube/draw.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		draw_program->attachShader("./shaders/marching_cube/draw.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		draw_program->setupDescriptorSetLayout({
			infos::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1)
		});
		draw_program->createDescriptorPool({
		 	infos::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
		});
		draw_program->allocDescriptorSet(&uniform_matrix.desc_set, 0, 1);
		auto attributes = Vertex::vertexInputAttributes();
		auto bindings = Vertex::vertexInputBinding();
		draw_program->graphics.vertex_input = infos::vertexInputStateCreateInfo(attributes, bindings);
		draw_program->build(render_pass, cache);

		draw_program->uniformUpdate(
				uniform_matrix.desc_set,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
				&mc.output.ubo.descriptor, nullptr
		);
	}

	void prepareCommandBuffer(){
		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clear_values[1].depthStencil = {1.0f, 0};
		draw_command_buffers.resize(swapchain.buffers.size());
		uint32_t sz_indices = mc.output.nr_faces *3;

		printf("sz_vertices : %d , sz indices : %d\n", mc.output.nr_vertices, sz_indices);

		VkRenderPassBeginInfo render_pass_BI = infos::renderPassBeginInfo();
		render_pass_BI.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_BI.pClearValues = clear_values.data();
		render_pass_BI.renderArea.offset = {0,0};
		render_pass_BI.renderArea.extent.height = height;
		render_pass_BI.renderArea.extent.width = width;
		render_pass_BI.renderPass = render_pass;

		for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			draw_command_buffers[i] = graphics_queue->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}
		
		for(uint32_t i = 0 ; i < draw_command_buffers.size() ; ++i){
			graphics_queue->beginCommandBuffer(draw_command_buffers[i]);
			render_pass_BI.framebuffer = framebuffers[i];
			VkViewport viewport = infos::viewport(static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
			VkRect2D scissor = infos::rect2D(width, height, 0, 0);
			vkCmdBeginRenderPass(draw_command_buffers[i], &render_pass_BI, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(draw_command_buffers[i], 0, 1, &viewport);
			vkCmdSetScissor(draw_command_buffers[i], 0, 1, &scissor);
			vkCmdBindPipeline(draw_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, draw_program->pipeline);
			VkBuffer vertex_buffer[] = {VkBuffer(mc.output.vertices)};
			VkBuffer indices_buffer[] = {VkBuffer(mc.output.indices)};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(draw_command_buffers[i], 0, 1, vertex_buffer ,offsets);
			vkCmdBindIndexBuffer(draw_command_buffers[i], indices_buffer[0], 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(draw_command_buffers[i], 
									VK_PIPELINE_BIND_POINT_GRAPHICS,
									draw_program->pipeline_layout, 
									0, 
									1, &uniform_matrix.desc_set,
									0, nullptr);
			vkCmdDrawIndexed(draw_command_buffers[i], sz_indices, 1, 0, 0, 0);
			vkCmdEndRenderPass(draw_command_buffers[i]);
			graphics_queue->endCommandBuffer(draw_command_buffers[i]);
		}	
	}

	void updateUniform(){
		uniform_matrix.model = glm::rotate(uniform_matrix.model,glm::radians(3.0f), glm::vec3(0.0f, 1.0, 0.0f));
		//uniform_matrix.proj[1][1] *= -1;
		mc.output.ubo.copyFrom(&uniform_matrix, sizeof(UniformMatrices));
	}

	public:
	void runMarchingCube(){
		std::chrono::time_point start = std::chrono::system_clock::now(); 
		PROFILING(mc.edgeTest(), "edgeTest()");
		PROFILING(mc.edgeTestPrefixSum(),"edgeTestPrefixSum()");
		PROFILING(mc.edgeCompact(), "edgeCompact()");
		PROFILING(mc.cellTest(), "cellTest()");
		PROFILING(mc.cellTestPrefixSum(),"cellTestPrefixSum()");
		PROFILING(mc.generateVertices(),"generateVertices()");
		PROFILING(mc.generateIndices(),"generateIndices()");
		std::chrono::duration<double> t = std::chrono::system_clock::now() - start;
		printf("Marching Cube spent %.4lf seconds\n", t.count()); 
		mc.save();
	}

	void imageTest(){
		float data[48] = {0.0f};

		LOG("image test start\n");
		Image image(context);
		image.createImage(4, 4, 3 ,VK_IMAGE_TYPE_3D, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, 1, 1);
		LOG("image create done\n");
		image.alloc(4*4*3*sizeof(float), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		LOG("memory allocation done\n");
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;
		image.bind(0);
		LOG("image memory bind done\n");
		VK_CHECK_RESULT(image.createImageView(VK_IMAGE_VIEW_TYPE_3D, range));
		LOG("image view create done\n");
		VkSamplerCreateInfo sampler_CI = infos::samplerCreateInfo();
		sampler_CI.compareOp = VK_COMPARE_OP_NEVER;
		sampler_CI.magFilter = VK_FILTER_NEAREST;
		sampler_CI.minFilter = VK_FILTER_NEAREST;
		sampler_CI.minLod = 0.0f;
		sampler_CI.maxLod = 1.0f;
		sampler_CI.mipLodBias = 0.0f;
		sampler_CI.maxAnisotropy = context->device_properties.limits.maxSamplerAnisotropy;
		sampler_CI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		sampler_CI.unnormalizedCoordinates = VK_FALSE;
		VK_CHECK_RESULT(image.createSampler(&sampler_CI));

		image.copyFrom(data, sizeof(float)*4*4*3);
		image.setupDescriptor();
		LOG("sampler create done\n");
		LOG("image test done\n");
	}

	void run(){
		Application::init();
		//prepareCompute();
		//runMarchingCube();
		//preparePrograms();
		//prepareCommandBuffer();
		//mainLoop();
		imageTest();
		destroy();
	}

	void destroy(){
		mc.destroy();
	}
};

int main(int argc, const char *argv[])	
{
	vector<const char*> instance_extensions(getRequiredExtensions());
	vector<const char *> validations={"VK_LAYER_KHRONOS_validation"};
	vector<const char *> device_extensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	string _name = "vulkan";
	string engine_name = "engine";
	string file_path = "assets/dragon_vrip_FLT32_128_128_64.raw";
	Volume.file_path = file_path;
	Volume.size = {128,128,64};
	Volume.isovalue = 0.0f;
	Volume.data = new float[Volume.size.x * Volume.size.y * Volume.size.z];
	loadVolume(file_path, Volume.data);
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
