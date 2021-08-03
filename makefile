CC=g++
CFLAGS += -std=c++17 -DDEBUG -DVKENGINE_GRAPHICS_MODE -DGLFW_INCLUDE_VULKAN
LDFLAGS += -lvulkan -ldl -lpthread -lglfw -I/usr/local/include -L/usr/local/lib
DEPENDENCIES += vk_engine.o vk_context.o vk_utils.o vk_infos.o vk_queue_family.o vk_application.o vk_queue.o vk_swapchain.o vk_framebuffer.o vk_buffer.o vk_program.o
TARGET=app
TEST=test

$(TEST) : $(DEPENDENCIES)
	$(CC) $(CFLAGS) -o test test.cpp $^ $(LDFLAGS)

vk_engine.o : vk_engine.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_context.o : vk_context.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_infos.o : vk_infos.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_utils.o : vk_utils.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_queue_family.o : vk_queue_family.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_queue.o : vk_queue.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_application.o : vk_application.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_swapchain.o : vk_swapchain.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_framebuffer.o : vk_framebuffer.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_buffer.o : vk_buffer.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

vk_program.o : vk_program.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)


clean:
	rm *.o 
	rm $(TEST)
	rm *.gfxr
