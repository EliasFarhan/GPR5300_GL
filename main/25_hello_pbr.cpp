//
// Created by efarhan on 5/3/19.
//

#include <engine.h>
#include <graphics.h>
#include <geometry.h>
#include <light.h>

#include <imgui.h>

class HelloPBRDrawingProgram : public DrawingProgram
{
public:
	void Init() override;
	void Draw() override;
	void Destroy() override;
	void UpdateUi() override;
private:
    Sphere sphere;
    Cube cube;
    Shader pbrShader;
    Shader pbrMapShader;

    float albedoColor[3] = {1,0,1};

    float ao =1.0f;
    float lightIntensity = 300;
    PointLight pointLight[4];

    unsigned albedoMap;
    unsigned normalMap;
    unsigned metallicMap ;
    unsigned roughMap;
    unsigned aoMap;
};

void HelloPBRDrawingProgram::Init()
{
    programName = "Hello PBR Basics";
    auto* engine = Engine::GetPtr();
    auto& config = engine->GetConfiguration();
    auto& camera = engine->GetCamera();

    camera.Position = glm::vec3(0.0f, 2.5f, 5.0f);

    pbrShader.CompileSource(
            "shaders/25_hello_pbr/pbr.vert",
            "shaders/25_hello_pbr/pbr.frag");
    pbrMapShader.CompileSource(
            "shaders/25_hello_pbr/pbr.vert",
            "shaders/25_hello_pbr/pbr_map.frag");
    shaders.push_back(&pbrShader);
    shaders.push_back(&pbrMapShader);
    pointLight[0].position = glm::vec3(10.0f,10.0f,10.0f);
    pointLight[1].position = glm::vec3(-10.0f,10.0f,10.0f);
    pointLight[2].position = glm::vec3(10.0f,10.0f,-10.0f);
    pointLight[3].position = glm::vec3(10.0f,-10.0f,10.0f);

    sphere.Init();
    cube.Init();

    aoMap = stbCreateTexture("data/textures/PavingStones/PavingStones42_AO.jpg");
    normalMap = stbCreateTexture("data/textures/PavingStones/PavingStones42_nrm.jpg");
    albedoMap = stbCreateTexture("data/textures/PavingStones/PavingStones42_col.jpg");
    roughMap = stbCreateTexture("data/textures/PavingStones/PavingStones42_rgh.jpg");

    char data[3] = {0,0,0};
    glGenTextures(1, &metallicMap);

    glBindTexture(GL_TEXTURE_2D, metallicMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1,1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void HelloPBRDrawingProgram::Draw()
{
    ProcessInput();
    glEnable(GL_DEPTH_TEST);
    auto* engine = Engine::GetPtr();
    auto& camera = engine->GetCamera();
    auto& config = engine->GetConfiguration();


    glm::mat4 projection = glm::perspective(
            camera.Zoom,
            (float)config.screenWidth / config.screenHeight,
            0.1f, 100.0f);

    pbrShader.Bind();
    for(int i = 0; i < 4; i++)
    {
        pointLight[i].color = glm::vec3(1.0f)*lightIntensity;
        pointLight[i].Bind(pbrShader, i);
    }
    pbrShader.SetInt("pointLightsNmb", 4);
    pbrShader.SetMat4("view", camera.GetViewMatrix());
    pbrShader.SetMat4("projection", projection);
    pbrShader.SetVec3("albedo", albedoColor);

    pbrShader.SetFloat("ao", ao);
    pbrShader.SetVec3("camPos", camera.Position);
    glm::mat4 model;
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5;j++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i, j, 0));

            model = glm::scale(model, glm::vec3(0.25f));
            pbrShader.SetFloat("metallic", i/5.0f);
            pbrShader.SetFloat("roughness", j/5.0f);
            pbrShader.SetMat4("model", model);
            sphere.Draw();
        }
    }
    pbrMapShader.Bind();
    for(int i = 0; i < 4; i++)
    {
        pointLight[i].color = glm::vec3(1.0f,1.0f,1.0f)*lightIntensity;
        pointLight[i].Bind(pbrMapShader, i);
    }
    pbrMapShader.SetInt("pointLightsNmb", 4);
    pbrMapShader.SetMat4("view", camera.GetViewMatrix());
    pbrMapShader.SetMat4("projection", projection);

    pbrMapShader.SetInt("albedoMap", 0);
    pbrMapShader.SetInt("normalMap", 1);
    pbrMapShader.SetInt("metallicMap", 2);
    pbrMapShader.SetInt("roughnessMap", 3);
    pbrMapShader.SetInt("aoMap", 4);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallicMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, roughMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, aoMap);
    pbrMapShader.SetVec3("camPos", camera.Position);
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5;j++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i-5, j, 0));

            model = glm::scale(model, glm::vec3(0.25f));

            pbrMapShader.SetMat4("model", model);
            cube.Draw();
        }
    }

}

void HelloPBRDrawingProgram::Destroy()
{
}

void HelloPBRDrawingProgram::UpdateUi()
{
    ImGui::Separator();
    ImGui::ColorEdit3("albedo", albedoColor);
    ImGui::SliderFloat("lightColor", &lightIntensity, 1.0f, 300.0f);
    ImGui::SliderFloat("ao", &ao, 0.0f, 1.0f);
}


int main(int argc, char** argv)
{
	Engine engine;
	srand(0);
	auto& config = engine.GetConfiguration();
	config.screenWidth = 1280;
	config.screenHeight = 720;
	config.windowName = "Hello PBR";
	config.bgColor = { 1,1,1,0 };
	engine.AddDrawingProgram(new HelloPBRDrawingProgram());
	engine.Init();
	engine.GameLoop();
	return EXIT_SUCCESS;
}
