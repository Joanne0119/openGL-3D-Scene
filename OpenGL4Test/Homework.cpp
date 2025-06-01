// ✓ (2分)滑鼠可以控制鏡頭的方向
// ✓ (3分)上下左右四個鍵可以控制前進與後退(以下二選一)
//    (3分) 鏡頭方向
//    (1分) 世界座標方向
// (3分) 依照需求完成房間的東西放置
// 房間正中間必須放置一個模型(必須是讀自 Obj 檔)
// 另外三個 Spot Light 處也必須各至少有一個模型
// 每一個模型都必須有材質的設定(可以不含貼圖)
// (3分) 依照需求完成燈光的放置，而且都會照亮環境
// 也就是每一個物件必須能接受兩個以上光源的照明計算加總
// 主燈光跟各自的 Spot Light
// 三個光源(標示紅點處)必須是 Spot Light，而且有其各自照明方向跟控制
// (2分) 2D介面呈現在畫面上
// (2分) 2D介面所對應的四個功能都有作用，一個功能0.5分
// 介面的類別封裝部分沒有特別的需求
// (2分) 正中間的主燈光必須使用到 PerPixel Lighting
// (2分) 不會穿牆
// (1分) 至少有一個光源搭配鍵盤的 R G B 提供顏色的改變
// (5%) 創意分數，自由發揮非上述功能

//#define GLM_ENABLE_EXPERIMENTAL 1

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/string_cast.hpp>

#include "common/initshader.h"
#include "common/arcball.h"
#include "common/wmhandler.h"
#include "common/CCamera.h"
#include "common/CShaderPool.h"
#include "common/CButton.h"
#include "models/CQuad.h"
#include "models/CBottle.h"
#include "models/CTeapot.h"
#include "models/CTorusKnot.h"
#include "models/CBox.h"
#include "models/CSphere.h"

//#include "common/OBJLoader.h"
//#include "common/ModelManager.h"
//#include "common/ModelLoader.h"
#include "Model.h"


#include "common/CLight.h"
#include "common/CMaterial.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 800 
#define ROW_NUM 30

CBottle  g_bottle(18, 8, 4);
CTeapot  g_teapot(5);
CTorusKnot g_tknot(4);
CBox g_house;
CSphere g_sphere;
CButton g_button;

glm::vec3 g_eyeloc(6.0f, 6.0f, 6.0f); // 鏡頭位置, 預設在 (8,8,8) 
CCube g_centerloc; // view center預設在 (0,0,0)，不做任何描繪操作
CQuad g_floor[ROW_NUM][ROW_NUM]; 

GLuint g_shadingProg;
GLuint g_objShadingProg;
GLuint g_modelVAO;
int g_modelVertexCount;

// 全域光源 (位置在 5,5,0)
CLight g_light(glm::vec3(5.0f, 5.0f, 0.0f));

// 全域材質（可依模型分別設定）
CMaterial g_matBeige;   // 淺米白深麥灰
CMaterial g_matGray;    //  深麥灰材質
CMaterial g_matWaterBlue;
CMaterial g_matWaterGreen;
CMaterial g_matWaterRed;
CMaterial g_matWoodHoney;
CMaterial g_matWoodLightOak;
CMaterial g_matWoodBleached;

//ModelManager g_modelManager;
//ModelLoader model;
//Model myModel;
std::vector<std::unique_ptr<Model>> models;

std::vector<std::string> modelPaths = {
    "models/woodCube.obj",
    "models/Elephant_Toy.obj"
};

void genMaterial();
void renderModel(const std::string& modelName, const glm::mat4& modelMatrix);

//----------------------------------------------------------------------------
void loadScene(void)
{
    genMaterial();
    g_shadingProg = CShaderPool::getInstance().getShader("v_phong.glsl", "f_phong.glsl");
//    g_objShadingProg = CShaderPool::getInstance().getShader("v_obj.glsl", "f_obj.glsl");
    
    g_light.setShaderID(g_shadingProg, "uLight");
    //g_light.setTarget(glm::vec3(0, 2, 0));
    //g_light.setCutOffDeg(20.0f, 90.0f);
    //g_light.setCutOffDeg(20.0f, 90.0f, 8.0f);

    int k = 0;
    for (int i = 0; i < ROW_NUM; i++)
    {
        k++;
        for (int j = 0; j < ROW_NUM; j++) {
            g_floor[i][j].setupVertexAttributes();
            g_floor[i][j].setShaderID(g_shadingProg, 3);
            g_floor[i][j].setPos(glm::vec3((ROW_NUM / 2) - 0.5f - (float)i, 0.0f, (float)j - (ROW_NUM / 2) + 0.5f));
            g_floor[i][j].setRotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            if (k % 2) g_floor[i][j].setMaterial(g_matBeige);
            else g_floor[i][j].setMaterial(g_matGray);
            k++;
        }
    }

    g_bottle.setupVertexAttributes();
    g_bottle.setShaderID(g_shadingProg, 3);
    g_bottle.setScale(glm::vec3(0.7f, 0.7f, 0.7f));
    g_bottle.setPos(glm::vec3(2.5f, 0.0005f, 2.5f));
    g_bottle.setMaterial(g_matWaterBlue);

    g_teapot.setupVertexAttributes();
    g_teapot.setShaderID(g_shadingProg, 3);
    g_teapot.setScale(glm::vec3(0.75f, 0.75f, 0.75f));
    g_teapot.setPos(glm::vec3(-3.0f, 0.0005f, -3.0f));
    g_teapot.setRotate(45, glm::vec3(0, 1, 0));
    g_teapot.setMaterial(g_matWaterGreen);
    
    g_tknot.setupVertexAttributes();
    g_tknot.setShaderID(g_shadingProg, 3);
    g_tknot.setScale(glm::vec3(0.4f, 0.4f, 0.4f));
    g_tknot.setPos(glm::vec3(-2.0f, 0.5f, 2.0f));
    g_tknot.setMaterial(g_matWaterRed);

    g_house.setupVertexAttributes();
    g_house.setShaderID(g_shadingProg, 3);
    g_house.setScale(glm::vec3(30.0f, 12.0f, 30.0f));
    g_house.setPos(glm::vec3(0.0f, 5.95f, 0.0f));
    g_house.setMaterial(g_matWoodBleached);
    
    // 載入模型 - 只需要傳入模型路徑！
    for (const auto& path : modelPaths) {
        auto model = std::make_unique<Model>();
        if (model->LoadModel(path)) {
            models.push_back(std::move(model));
            std::cout << "Successfully loaded: " << path << std::endl;
        } else {
            std::cout << "Failed to load: " << path << std::endl;
        }
    }
   

	CCamera::getInstance().updateView(g_eyeloc); // 設定 eye 位置
    CCamera::getInstance().updateCenter(glm::vec3(0,4,0));
	CCamera::getInstance().updatePerspective(45.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);
    glm::mat4 mxView = CCamera::getInstance().getViewMatrix();
	glm::mat4 mxProj = CCamera::getInstance().getProjectionMatrix();

    GLint viewLoc = glGetUniformLocation(g_shadingProg, "mxView"); 	// 取得 view matrix 變數的位置
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(mxView));

    GLint projLoc = glGetUniformLocation(g_shadingProg, "mxProj"); 	// 取得投影矩陣變數的位置
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(mxProj));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 設定清除 back buffer 背景的顏色
    glEnable(GL_DEPTH_TEST); // 啟動深度測試
}
//----------------------------------------------------------------------------

void render(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 設定 back buffer 的背景顏色
    glUseProgram(g_shadingProg);

    //上傳光源與相機位置
    g_light.updateToShader();
    glUniform3fv(glGetUniformLocation(g_shadingProg, "viewPos"), 1, glm::value_ptr(g_eyeloc));
    glUniform3fv(glGetUniformLocation(g_shadingProg, "lightPos"), 1, glm::value_ptr(g_light.getPos()));

    for (int i = 0; i < ROW_NUM; i++)
        for (int j = 0; j < ROW_NUM; j++) {
            g_floor[i][j].uploadMaterial();
            g_floor[i][j].drawRaw();
        }

    g_light.drawRaw();
    g_bottle.uploadMaterial();
    g_bottle.drawRaw();
    g_teapot.uploadMaterial();
    g_teapot.drawRaw();
    g_tknot.uploadMaterial();
    g_tknot.drawRaw();

    g_house.uploadMaterial();
    g_house.drawRaw();
    
//    glUseProgram(g_objShadingProg);
    //繪製obj model
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));

    GLint modelLoc = glGetUniformLocation(g_shadingProg, "mxModel");
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        std::cout << "  Set mxModel uniform" << std::endl;
    } else {
        std::cerr << "  mxModel uniform not found!" << std::endl;
    }

    std::cout << "Calling myModel.Render(g_objShadingProg)..." << std::endl;
    for (auto& model : models) {
         model->Render(g_shadingProg);
     }
    std::cout << "myModel.Render() finished" << std::endl;

    // 檢查 OpenGL 錯誤 (after rendering)
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error in render(): " << error << std::endl;
    }
}
//----------------------------------------------------------------------------

void update(float dt)
{
    g_light.update(dt);
}

void releaseAll()
{
//    g_modelManager.cleanup();
}

int main() {
    // ------- 檢查與建立視窗  ---------------  
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 設定 OpenGL 版本與 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //只啟用 OpenGL 3.3 Core Profile（不包含舊版 OpenGL 功能）
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // 禁止視窗大小改變

    // 建立 OpenGL 視窗與該視窗執行時所需的的狀態、資源和環境(context 上下文)
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL_4 Example 4 NPR", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 設定將這個視窗的資源(OpenGL 的圖形上下文）與當前執行緒綁定，讓該執行緒能夠操作該視窗的資源
    glfwMakeContextCurrent(window);

    // 設定視窗大小, 這樣 OpenGL 才能知道如何將視窗的內容繪製到正確的位置
    // 基本上寬與高設定成視窗的寬與高即可
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    // ---------------------------------------

    // 設定相關事件的 callback 函式，以便事件發生時，能呼叫對應的函式
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);// 視窗大小被改變時
    glfwSetKeyCallback(window, keyCallback);                        // 有鍵盤的按鍵被按下時
    glfwSetMouseButtonCallback(window, mouseButtonCallback);        // 有滑鼠的按鍵被按下時
    glfwSetCursorPosCallback(window, cursorPosCallback);            // 滑鼠在指定的視窗上面移動時
	glfwSetScrollCallback(window, scrollCallback);			        // 滑鼠滾輪滾動時

    // 呼叫 loadScene() 建立與載入 GPU 進行描繪的幾何資料 
    loadScene();


    float lastTime = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastTime; // 計算前一個 frame 到目前為止經過的時間
        lastTime = currentTime;
        update(deltaTime);      // 呼叫 update 函式，並將 deltaTime 傳入，讓所有動態物件根據時間更新相關內容
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    releaseAll(); // 程式結束前釋放所有的資源
    glfwTerminate();
    return 0;
}

void genMaterial()
{
    // 設定材質
    g_matBeige.setAmbient(glm::vec4(0.0918f, 0.0906f, 0.0863f, 1.0f));
    g_matBeige.setDiffuse(glm::vec4(0.8258f, 0.8152f, 0.7765f, 1.0f));
    g_matBeige.setSpecular(glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
    g_matBeige.setShininess(32.0f);

    g_matGray.setAmbient(glm::vec4(0.0690f, 0.06196f, 0.05451f, 1.0f));
    g_matGray.setDiffuse(glm::vec4(0.6212f, 0.5576f, 0.4906f, 1.0f));
    g_matGray.setSpecular(glm::vec4(0.20f, 0.20f, 0.20f, 1.0f));
    g_matGray.setShininess(16.0f);

    g_matWaterBlue.setAmbient(glm::vec4(0.080f, 0.105f, 0.120f, 1.0f));
    g_matWaterBlue.setDiffuse(glm::vec4(0.560f, 0.740f, 0.840f, 1.0f));
    g_matWaterBlue.setSpecular(glm::vec4(0.20f, 0.20f, 0.20f, 1.0f));
    g_matWaterBlue.setShininess(32.0f);

    g_matWaterGreen.setAmbient(glm::vec4(0.075f, 0.120f, 0.090f, 1.0f));
    g_matWaterGreen.setDiffuse(glm::vec4(0.540f, 0.840f, 0.640f, 1.0f));
    g_matWaterGreen.setSpecular(glm::vec4(0.20f, 0.20f, 0.20f, 1.0f));
    g_matWaterGreen.setShininess(32.0f);

    g_matWaterRed.setAmbient(glm::vec4(0.125f, 0.075f, 0.075f, 1.0f));
    g_matWaterRed.setDiffuse(glm::vec4(0.860f, 0.540f, 0.540f, 1.0f));
    g_matWaterRed.setSpecular(glm::vec4(0.20f, 0.20f, 0.20f, 1.0f));
    g_matWaterRed.setShininess(32.0f);

    g_matWoodHoney.setAmbient(glm::vec4(0.180f, 0.164f, 0.130f, 1.0f));
    g_matWoodHoney.setDiffuse(glm::vec4(0.720f, 0.656f, 0.520f, 1.0f));
    g_matWoodHoney.setSpecular(glm::vec4(0.30f, 0.30f, 0.30f, 1.0f));
    g_matWoodHoney.setShininess(24.0f);

    g_matWoodLightOak.setAmbient(glm::vec4(0.200f, 0.180f, 0.160f, 1.0f));
    g_matWoodLightOak.setDiffuse(glm::vec4(0.800f, 0.720f, 0.640f, 1.0f));
    g_matWoodLightOak.setSpecular(glm::vec4(0.30f, 0.30f, 0.30f, 1.0f));
    g_matWoodLightOak.setShininess(24.0f);

    g_matWoodBleached.setAmbient(glm::vec4(0.220f, 0.215f, 0.205f, 1.0f));
    g_matWoodBleached.setDiffuse(glm::vec4(0.880f, 0.860f, 0.820f, 1.0f));
    g_matWoodBleached.setSpecular(glm::vec4(0.30f, 0.30f, 0.30f, 1.0f));
    g_matWoodBleached.setShininess(24.0f);
}
