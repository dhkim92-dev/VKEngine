CC=g++
CFLAGS += -std=c++17 -DDEBUG -DGLFW_INCLUDE_VULKAN
LDFLAGS += -lvulkan -ldl -lMoltenVK -lpthread -lglfw
DEPENDENCIES += vk_engine.o vk_context.o vk_utils.o vk_infos.o vk_queue_family.o vk_application.o
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

vk_application.o : vk_application.cpp
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

clean:
	rm *.o 
	rm $(TEST)
	rm *.gfxr
