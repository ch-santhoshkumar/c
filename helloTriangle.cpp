#include<algorithm>
#include<vulkan/vulkan.h>
#include<xcb/xcb.h>
#include"/home/pi/Downloads/mesa/include/vulkan/vulkan_xcb.h"
#include<cstdint>

#include<ctime>
#include<cstring>
#include<iostream>
#include<cstdlib>
#include<stdexcept>
#include<vector>
#include<optional>

class Triangle{
public:
	void run(){
		xcbWindow();
		initVulkan();
	//	mainLoop();
		cleanUp();
		}
private:
	VkInstance instance;
	VkPhysicalDevice physicalDevice=VK_NULL_HANDLE;
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures{};
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkDevice device;
	VkQueue queue;
	std::string title = "Vulkan Example";
	std::string name = "vulkanExample";
	xcb_window_t window;
	xcb_connection_t *connection;
	uint32_t frameCounter = 0;
	uint32_t lastFPS = 0;
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	int graphicsQueue=8;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	//Initializing vulkan
	void initVulkan(){
		createInstance();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		swapChainSupportDetails details=querySwapChainSupport();
		createSwapChain(details);
		createImageView();
	}
	
	
	void mainLoop(){
	}
	
	//Cleanup 
	void cleanUp(){
		//xcb_disconnect(connection);
		destroyImageview();
		destroySwapchain();
		destroyLogicalDevice();
		destroySurface();
		destroyInstance();
	}

	void createInstance(){
		uint32_t extensionCount=0, layerCount=0;
		//Application Info
		VkApplicationInfo appInfo{};
		
		std::vector<const char*> instanceExtensions;
		
		appInfo.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName="Hello Triangle";
		appInfo.applicationVersion=VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName="no engine";
		appInfo.engineVersion=VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion=VK_API_VERSION_1_0;
		//Checking available Extensions
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::cout << "Available Extensions count=" <<extensionCount <<"\n";
		std::vector<VkExtensionProperties> extensionList(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionList.data());	
		for(int i=0; i<extensionCount; i++)
		{
			std::cout << extensionList[i].extensionName << "\n";
			instanceExtensions.push_back(extensionList[i].extensionName );
		}

		vkEnumerateInstanceLayerProperties( &layerCount, nullptr);
		if(layerCount==0)
			std::cout<<"No Layers Available" <<"\n";
		std::vector<VkLayerProperties> layerList(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerList.data());	
		for(int i=0; i<layerCount; i++)
		{
			std::cout << layerList[i].layerName << "\n";
		}
		
		VkInstanceCreateInfo createInfo{};
		createInfo.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext=nullptr;
		createInfo.pApplicationInfo=&appInfo;
		createInfo.enabledExtensionCount=extensionCount;
		createInfo.ppEnabledExtensionNames=instanceExtensions.data();
		createInfo.enabledLayerCount=0;
		createInfo.ppEnabledLayerNames=nullptr;
		//Creating Instance
		if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
			throw std::runtime_error("Instance creation failed");
		}
		std::cout <<"SUCCESS: Instance created Successfully" << "\n";
	}
	
	void destroyInstance()
	{	
		vkDestroyInstance(instance, nullptr);
		std::cout <<"DESTROY: Instance destroyed Successfully" << "\n";	
	}

	void destroyLogicalDevice(){
		vkDestroyDevice(device, nullptr);
		std::cout <<"DESTROY: Logical device destroyed successfully" <<"\n";
	
	}
	void destroySurface(){
		vkDestroySurfaceKHR(instance, surface, nullptr);
		std::cout <<"DESTROY: Surface destroyed successfully"<< "\n";
	}

	void destroySwapchain(){
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		std::cout<<"DESTROY: Swapchain destroyed successfully"<<"\n";
	}

	void destroyImageview(){
		for(auto imageView : swapChainImageViews){
			vkDestroyImageView(device, imageView, nullptr);
		}
		std::cout<<"DESTROY: ImageViews destroyed successfully"<<"\n";

	}
	void createSurface(){
		VkXcbSurfaceCreateInfoKHR surfaceInfo{};
		surfaceInfo.sType=VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext=NULL;
		surfaceInfo.flags=0;
		surfaceInfo.connection=connection;
		surfaceInfo.window=window;
		if(vkCreateXcbSurfaceKHR(instance, &surfaceInfo, nullptr, &surface)!= VK_SUCCESS)
		{
			throw std::runtime_error("Surface creation error");
		}
		std::cout << "SUCCESS: Surface created successfully"<<"\n";
	}

	void pickPhysicalDevice(){
		uint32_t deviceCount=0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		std::cout <<"Available Devices="<< deviceCount <<"\n";
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		physicalDevice=devices[0];
		if(physicalDevice==VK_NULL_HANDLE)
			std::cout<<"NO device handle alloted" <<"\n";
		else{
			std::cout<<"Physical device handle="<<physicalDevice <<"\n";
		}
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		std::cout<<"Device Type="<<deviceProperties.deviceType << "\n";
		std::cout<<"Vendor ID=" <<deviceProperties.vendorID << "\n";
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		std::cout<<"Sample Rate Shading "<<deviceFeatures.sampleRateShading<<"\n";
		findQueueFamilies();
	}
	struct QueueFamilyIndices{
		uint32_t graphicsFamily;
		uint32_t presentFamily;
	};

	void findQueueFamilies()
	{
		VkBool32 support=VK_TRUE;
		uint32_t queueCount=0;
		int i=0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
		if(queueCount==0)
			throw std::runtime_error("No queue families available");
		std::cout <<"Available queue family count=" << queueCount <<"\n";
		std::vector<VkQueueFamilyProperties> queueList(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueList.data());
		for(auto const& queue : queueList){
			if(queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				std::cout <<"Graphics Queue is available" << "\n";
				graphicsQueue=i;
			}
			i++;
		}
		if(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsQueue, surface, &support)!= VK_SUCCESS)
		{
			throw std::runtime_error("ERROR: Index don't support presentation");
		}
		std::cout << "Presentation support="<<support<<"\n";
		std::cout <<"Index supports Presentation Queue" <<graphicsQueue<<"\n";
		std::cout<< "Graphics Queue index="<<graphicsQueue <<"\n";
	}

	struct swapChainSupportDetails{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentationModes;
	};

	swapChainSupportDetails  querySwapChainSupport()
	{
		bool swapChainSupport= false;
		uint32_t formatCount, presentModeCount;
		swapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		if(formatCount != 0){
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
		}
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
		if(presentModeCount != 0){
			details.presentationModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentationModes.data());
		}
		swapChainSupport = !details.formats.empty() && !details.presentationModes.empty();
		std::cout<<"_______________Device has swapchain Support="<<swapChainSupport<<"\n";
		return details;

	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
		for(const auto& availableFormat : availableFormats){
			if(availableFormat.format== VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		
		
		}
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){

		for(const auto& availablePresentMode : availablePresentModes)
		{
			if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				std::cout << "Mailbox presentation mode available"<<"\n";
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}
		std::cout<<"No Mailbox mode available Enableing FIFO Mode"<<"\n";
		return VK_PRESENT_MODE_FIFO_KHR;
	}
	
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if(capabilities.currentExtent.width != UINT32_MAX){
			return capabilities.currentExtent;
		}
		else{
			VkExtent2D actualExtent = {WIDTH, HEIGHT};

			actualExtent.width=std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height=std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}
	
	}

	void createSwapChain(swapChainSupportDetails details){
		uint32_t imageCount;
		std::cout<<"inside createSwapChain function" <<"\n";
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentationModes);
		VkExtent2D extent = chooseSwapExtent(details.capabilities);
		imageCount=details.capabilities.minImageCount+1;
		if(details.capabilities.maxImageCount>0 && imageCount>details.capabilities.maxImageCount)
		{
			imageCount=details.capabilities.maxImageCount;
		}
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
	       	createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount=0;
		createInfo.pQueueFamilyIndices=nullptr;	
		createInfo.preTransform = details.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode=presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("ERROR: Failed to create swapchain");	
		}
		std::cout<<"SUCCESS: Swapchain created successfully" <<"\n";
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void createImageView(){
		swapChainImageViews.resize(swapChainImages.size());
		for(size_t i=0; i < swapChainImages.size(); i++){
			VkImageViewCreateInfo createInfo{};
			createInfo.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType =VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format=swapChainImageFormat;	
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;	
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;		
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;	
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;	
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("ERROR: ImageView Creation Failed");
			}
		}
		std::cout<<"SUCCESS: ImageViews created successfully" <<"\n";
	}

	void createLogicalDevice(){
		uint32_t deviceExtensionCount=0;
		float queuePriority=0.1f;
		//char swapchainEx = VK_KHR_swapchain ;
		const std::vector<const char*>Ex={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		//Ex.push_back(&swapchainEx);
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext=nullptr;	 
		queueCreateInfo.queueFamilyIndex=graphicsQueue;	
		queueCreateInfo.queueCount=1;	
		queueCreateInfo.pQueuePriorities=&queuePriority;
		
		VkDeviceCreateInfo deviceInfo{};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext=nullptr;
		deviceInfo.queueCreateInfoCount=1;
		deviceInfo.pQueueCreateInfos=&queueCreateInfo;
		deviceInfo.enabledLayerCount=0;
		deviceInfo.ppEnabledLayerNames=nullptr;
		deviceInfo.enabledExtensionCount=1;
		deviceInfo.ppEnabledExtensionNames=Ex.data();
		deviceInfo.pEnabledFeatures=&deviceFeatures;
		//Create logical device
		if(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("Logical device creation failed");
		}
		else{
			std::cout <<"SUCCESS: Logical Device created successfully"<<"\n";
		}
		//Checking Device extension properties
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);
		std::cout<<"Available Device Extensions="<<deviceExtensionCount<<"\n";
		std::vector<VkExtensionProperties> deviceExtensionList(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, deviceExtensionList.data());
		for(const auto& list: deviceExtensionList)
		{
			std::cout<<list.extensionName<<"\n";
		}
		//queue handle 
		vkGetDeviceQueue(device, graphicsQueue, 0, &queue);
		std::cout <<"Queue handle obtained successfully"<<"\n";
		std::cout<<"Queue handle=" << queue <<"\n";	
	
	
	}

	struct Settings {
		/** @brief Activates validation layers (and message output) when set to true */
		bool validation = false;
		/** @brief Set to true if fullscreen mode has been requested via command line */
		bool fullscreen = false;
		/** @brief Set to true if v-sync will be forced for the swapchain */
		bool vsync = false;
		/** @brief Enable UI overlay */
		bool overlay = false;
	} settings;
	
	static inline xcb_intern_atom_reply_t* intern_atom_helper(xcb_connection_t *conn, bool only_if_exists, const char *str)
	{
		xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
		
		xcb_intern_atom_reply_t* x =xcb_intern_atom_reply(conn, cookie, NULL);
		std::cout << (*x).response_type<<"\n" ;
	       	return x;	
	}
	
	std::string getWindowTitle()
	{
		std::string device(deviceProperties.deviceName);
		std::string windowTitle;
		windowTitle = title + " - " + device;
		if (!settings.overlay) {
			windowTitle += " - " + std::to_string(frameCounter) + " fps";
		}
		return windowTitle;
	}
	void  xcbWindow()
	{
		uint32_t value_mask, value_list[2];
		uint16_t width, height;
	
		xcb_intern_atom_reply_t *atom_wm_delete_window;
		xcb_screen_t *screen;
		connection= xcb_connect(NULL, NULL);
		screen = xcb_setup_roots_iterator (xcb_get_setup (connection)).data;
		window = xcb_generate_id(connection);

		value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		value_list[0] = screen->black_pixel;
		value_list[1] =
			XCB_EVENT_MASK_KEY_RELEASE |
			XCB_EVENT_MASK_KEY_PRESS |
			XCB_EVENT_MASK_EXPOSURE |
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |
			XCB_EVENT_MASK_POINTER_MOTION |
			XCB_EVENT_MASK_BUTTON_PRESS |
			XCB_EVENT_MASK_BUTTON_RELEASE;

		
		if (settings.fullscreen)
		{
			width  = screen->width_in_pixels;
			height = screen->height_in_pixels;
		}
		
		xcb_create_window(connection,
			XCB_COPY_FROM_PARENT,
			window, screen->root,
			0, 0, 150, 150, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			screen->root_visual,
			value_mask, value_list);

		/* Magic code that will send notification when window is destroyed */
			
	 	xcb_intern_atom_reply_t* reply = intern_atom_helper(connection, true, "WM_PROTOCOLS");
		atom_wm_delete_window = intern_atom_helper(connection, false, "WM_DELETE_WINDOW");
		std::cout<< (*reply).response_type<< "\n";
		
		
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
			window, (*reply).atom, 4, 32, 1,
			&(*atom_wm_delete_window).atom);

		std::string windowTitle = getWindowTitle();
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
			window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
			title.size(), windowTitle.c_str());

		free(reply);
		
		/*
		* Set the WM_CLASS property to display
	 	* title in dash tooltip and application menu
	 	* on GNOME and other desktop environments
	 	*/
		
		std::string wm_class;
		wm_class = wm_class.insert(0, name);
		wm_class = wm_class.insert(name.size(), 1, '\0');
		wm_class = wm_class.insert(name.size() + 1, title);
		wm_class = wm_class.insert(wm_class.size(), 1, '\0');
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, wm_class.size() + 2, wm_class.c_str());
		
		
		if (settings.fullscreen)
		{
			xcb_intern_atom_reply_t *atom_wm_state = intern_atom_helper(connection, false, "_NET_WM_STATE");
			xcb_intern_atom_reply_t *atom_wm_fullscreen = intern_atom_helper(connection, false, "_NET_WM_STATE_FULLSCREEN");
			xcb_change_property(connection,
					XCB_PROP_MODE_REPLACE,
					window, atom_wm_state->atom,
					XCB_ATOM_ATOM, 32, 1,
					&(atom_wm_fullscreen->atom));
			free(atom_wm_fullscreen);
			free(atom_wm_state);
		}
		
		xcb_map_window(connection, window);	
		//xcb_flush(connection);			
	}	
	
};

int main(){
	Triangle app;
	try{
		app.run();
	}catch(const std::exception& e){
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
