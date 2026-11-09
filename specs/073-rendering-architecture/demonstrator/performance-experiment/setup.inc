        require(SDL_Init(SDL_INIT_VIDEO)==0,SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
        auto *window=SDL_CreateWindow("073 SkParagraph | performance measurement",
            SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,
            SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI|(vk?SDL_WINDOW_VULKAN:SDL_WINDOW_OPENGL));
        require(window,SDL_GetError());
        SDL_GLContext gl=nullptr;sk_sp<GrDirectContext> context;
        std::unique_ptr<skwindow::internal::VulkanWindowContext> vulkan;
        std::string device;
        if(vk) {
            auto proc=reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr());
            require(proc,"Vulkan loader missing");
            auto create=[&](VkInstance instance){VkSurfaceKHR surface;require(SDL_Vulkan_CreateSurface(window,instance,&surface),SDL_GetError());return surface;};
            auto present=[&](VkInstance instance,VkPhysicalDevice physical,uint32_t queue) {
                auto surface=create(instance);
                auto support=reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(proc(instance,"vkGetPhysicalDeviceSurfaceSupportKHR"));
                auto destroy=reinterpret_cast<PFN_vkDestroySurfaceKHR>(proc(instance,"vkDestroySurfaceKHR"));
                auto properties=reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(proc(instance,"vkGetPhysicalDeviceProperties"));
                VkBool32 ok=false;support(physical,queue,surface,&ok);destroy(instance,surface,nullptr);
                VkPhysicalDeviceProperties p;properties(physical,&p);device=p.deviceName;return bool(ok);
            };
            vulkan=std::make_unique<skwindow::internal::VulkanWindowContext>(std::make_unique<skwindow::DisplayParams>(),create,present,proc);
            require(vulkan->isValid(),"Skia Vulkan initialization failed");
        } else {
            gl=SDL_GL_CreateContext(window);require(gl,SDL_GetError());SDL_GL_SetSwapInterval(1);
            context=GrDirectContexts::MakeGL(GrGLMakeAssembledInterface(nullptr,[](void *,const char *name){return reinterpret_cast<GrGLFuncPtr>(SDL_GL_GetProcAddress(name));}));require(bool(context),"Skia GL initialization failed");
            device=reinterpret_cast<const char *>(glGetString(GL_RENDERER));
        }
