#include"PathTrace.h"

namespace PathTrace {
    
    

    Scene* scene = nullptr;
    Renderer* renderer = nullptr;
    std::vector<string> sceneFiles;
    std::vector<string> envMaps;
    int sampleSceneIdx = 0;
    int selectedInstance = -1;
    int envMapIdx = 0;
    float CameraMoveSpeed = 3.0f;
    bool objectPropChanged=false;
    bool showTransform=false;
    bool space_down = false;
    float screenX[2] = {0,0};
    float screenY[2] = { 0,0 };

    std::string shadersDir = "../../../res/PathTrace/shaders/";
    std::string assetsDir = "../../../res/PathTrace/Scenes/ObjNor";
    std::string envMapDir = "../../../res/PathTrace/Scenes/HDR/";

    RenderOptions renderOptions;

    Scene* GetScene() {

        return scene;
    }
    Renderer* GetRenderer() {
        return renderer;
    }
    RenderOptions& GetRenderOptions() {
        return renderOptions;
    }
    void GetSceneFiles()
    {
        std::filesystem::directory_entry p_directory(assetsDir);
        for (auto& item : std::filesystem::directory_iterator(p_directory))
            if (!item.is_directory()) {
                auto ext = item.path().extension();
                if (ext == ".scene")
                {
                    sceneFiles.push_back(item.path().generic_string());
                }
            }
    }
    void GetEnvMaps()
    {
        std::filesystem::directory_entry p_directory(envMapDir);
        for (auto& item : std::filesystem::directory_iterator(p_directory)) {
            if (item.path().extension() == ".hdr")
            {
                envMaps.push_back(item.path().generic_string());

            }
        }
    }

    void LoadScene(std::string sceneName)
    {
        delete scene;
        scene = new Scene();
        std::string ext = sceneName.substr(sceneName.find_last_of(".") + 1);

        bool success = false;
        Mat4 xform;

        if (ext == "scene")
            success = LoadSceneFromFile(sceneName, scene, renderOptions);

        if (!success)
        {
            printf("Unable to load scene\n");
            exit(0);
        }

        selectedInstance = 0;
        // Add a default HDR if there are no lights in the scene
        if (!scene->envMap && !envMaps.empty())
        {
            scene->AddEnvMap(envMaps[envMapIdx]);
            renderOptions.enableEnvMap =  true ;
            renderOptions.envMapIntensity = 1.5f;
        }

        scene->renderOptions = renderOptions;
    }
    bool InitRenderer()
    {
        delete renderer;
        renderer = new Renderer(scene, shadersDir);
        return true;
    }
    void Ret() {
        delete renderer;
        delete scene;
    }
    
}