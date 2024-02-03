#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <irrKlang/irrKlang.h>
#include <ft2build.h>
#include FT_FREETYPE_H


#include "PlayerCamera.h"
#include "Model.h"
#include "Shader_m.h"
#include "Bullet.h"
#include "Target.h"


#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace irrklang;

// settings
const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 900;
ISoundEngine* engine = createIrrKlangDevice();

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

int score = 0;
bool wireframeOn = false;
bool shot = false;

PlayerCamera cam;
float yawWeapon;
float pitchWeapon;

float lastX = SCREEN_WIDTH / 2;
float lastY = SCREEN_HEIGHT / 2;
bool firstMouseMove = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::vector<Bullet*> bullets;
std::vector<Target*> targets;
std::map<char, Character> Characters;

glm::vec4 gunPos(9.198509f, 0.349500f, 3.725448f, 0.00f); // On spawn
glm::vec4 bulletPos;
glm::vec4 gunOffsetFromCam(0.296998f, -0.1505f, -0.054f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double posX, double posY);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

unsigned int loadTexture(const char* path);
void renderText(Shader shader, std::string text);
void updateGunPosForRotate();
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int textVAO, unsigned int textVBO);
bool checkCollison(Bullet * a);
float getPoint(float n1, float n2, float perc);


void generateWallVertices(std::vector<glm::vec3>& vec);
void generateTegelsVertices(std::vector<glm::vec3>& vec);
void generateFencePos(std::vector<glm::vec4>& vec);
void drawWalls();
void drawTegels();
void renderWalls(Shader brickShader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, std::vector<glm::vec3> cubePositions, unsigned int brickTexture);
void renderTegels(Shader tegelShader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, std::vector<glm::vec3> tegelsPostions, unsigned int tegelTexture);
void renderGun(Shader weaponArmShader, glm::mat4 model, glm::mat4 view, glm::mat4 projection,Model gunModel);
void renderBullets(Shader bulletShader,Model bulletModel, glm::mat4 view, glm::mat4 projection);
void renderTargets(Shader targetShader,Model targetModel, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
void renderFences(Shader fenceShader, Model fenceModel, glm::mat4 model, glm::mat4 view, glm::mat4 projection, std::vector<glm::vec4> fencePos);
void renderCrosshair();
int main(void)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Aimlabs - Melih version", NULL, NULL);

    // START SETUP, CLOSE BLOCK IF YOU DONT WANT TO SEE
    {
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);

        // tell GLFW to capture our mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!engine)
            return 0; // error starting up the engine

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        // configure global opengl state
        // -----------------------------
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

        /////// TEXT RENDEREING
        FT_Library ft;
        // All functions return a value different than 0 whenever an error occurred
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return -1;
        }

        // find path to font
        std::string font_name = "resources/fonts/arial.ttf";
        if (font_name.empty())
        {
            std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
            return -1;
        }

        // load font as face
        FT_Face face;
        if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return -1;
        }
        else {
            // set size to load glyphs as
            FT_Set_Pixel_Sizes(face, 0, 48);

            // disable byte-alignment restriction
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            // load first 128 characters of ASCII set
            for (unsigned char c = 0; c < 128; c++)
            {
                // Load character glyph 
                if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                {
                    std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                    continue;
                }
                // generate texture
                unsigned int texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
                );
                // set texture options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // now store character for later use
                Character character = {
                    texture,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<unsigned int>(face->glyph->advance.x)
                };
                Characters.insert(std::pair<char, Character>(c, character));
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }
    // SETUP DONE;

    // Shaders
    Shader textShader("resources/shaders/text.vs", "resources/shaders/text.fs");
    Shader brickShader("resources/shaders/brick.vs", "resources/shaders/brick.fs");
    Shader tegelShader("resources/shaders/tegel.vs", "resources/shaders/tegel.fs");
    Shader crossShader("resources/shaders/crosshair.vs", "resources/shaders/crosshair.fs");
    Shader weaponArmShader("resources/shaders/modelLoading.vs", "resources/shaders/modelLoading.fs");
    Shader bulletShader("resources/shaders/modelLoading.vs", "resources/shaders/modelLoading.fs");
    Shader targetShader("resources/shaders/target.vs", "resources/shaders/target.fs");
    Shader fenceShader("resources/shaders/fence.vs", "resources/shaders/fence.fs");

    glm::vec3 p0(5.0, 0.500000, 5.8);
    glm::vec3 p1(0, 0.500000, 3.3);
    glm::vec3 p2(5.0, 0.500000, 0.8);
    float za, xa, zb, xb,x,z;
    for (float i = 0; i < 1; i += 0.01)
    {
        xa = getPoint(p0.x, p1.x, i);
        xb = getPoint(p1.x, p2.x, i);
        za = getPoint(p0.z, p1.z, i);
        zb = getPoint(p1.z, p2.z, i);
        
        x = getPoint(xa, xb, i);
        z = getPoint(za, zb, i);
        
    }

    std::vector<glm::vec3> cubePositions;
    std::vector<glm::vec3> tegelsPostions;
    std::vector<glm::vec4> fencePos;
    generateTegelsVertices(tegelsPostions);
    generateWallVertices(cubePositions);
    generateFencePos(fencePos);

    Model gunModel("resources/objects/glockArms/glockArms.obj");
    Model bulletModel("resources/objects/glockArms/bullet1.obj");
    Model targetModel("resources/objects/target/target.obj");
    Model fenceModel("resources/objects/fence/fence.obj");

    unsigned int brickTexture = loadTexture("resources/textures/brickwall.jpg");
    unsigned int tegelTexture = loadTexture("resources/textures/marble.jpg");

    // Op dit moment geef ik de coordinaten want ik heb geen tijd meer om een algoritme daarvoor te shcrijven.
    targets.push_back(new Target(glm::vec3(5.032726, 0.500000, 5.871095), glm::vec3(3.683667, 0.500000, 4.364244), glm::vec3(5.385769, 0.500000, 2.100338)));
    targets.push_back(new Target(glm::vec3(5.460943, 0.500000, 3.619355), glm::vec3(3.207835, 0.500000, 5.502854), glm::vec3(1.111137, 0.500000, 4.940901)));
    targets.push_back(new Target(glm::vec3(1.729732, 0.500000, 1.234353), glm::vec3(6.532162, 0.500000, 3.545836), glm::vec3(0.928281, 0.500000, 5.603605)));

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bulletPos = glm::vec4(cam.playerPosition, 0.0f);

        glm::mat4 projection = glm::perspective((float)glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = cam.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        renderWalls(brickShader, model, view, projection, cubePositions, brickTexture);
        renderTegels(tegelShader, model, view, projection, tegelsPostions, tegelTexture);
        renderGun(weaponArmShader, model, view, projection, gunModel);
        renderBullets(bulletShader,bulletModel,view,projection);
        renderTargets(targetShader,targetModel,model,view,projection);
        renderFences(fenceShader,fenceModel,model,view,projection,fencePos);

        crossShader.use();
        renderCrosshair();

        textShader.use();
        std::string text = "Score: "+to_string(score);
        renderText(textShader, text);

        if (targets.size() == 0) {
            targets.push_back(new Target(glm::vec3(5.032726, 0.500000, 5.871095), glm::vec3(3.683667, 0.500000, 4.364244), glm::vec3(5.385769, 0.500000, 2.100338)));
            targets.push_back(new Target(glm::vec3(5.460943, 0.500000, 3.619355), glm::vec3(3.207835, 0.500000, 5.502854), glm::vec3(1.111137, 0.500000, 4.940901)));
            targets.push_back(new Target(glm::vec3(1.729732, 0.500000, 1.234353), glm::vec3(6.532162, 0.500000, 3.545836), glm::vec3(0.928281, 0.500000, 5.603605)));
        }   
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    engine->drop();
    return 0;
}


// Variables not in function because otherwise the vertices are getting declared again for no reason.
unsigned int tegelsVAO = 0;
unsigned int tegelsVBO = 0;
unsigned int crosshairVAO = 0;
unsigned int crosshairVBO = 0;
unsigned int wallsVAO = 0;
unsigned int wallsVBO = 0;

void drawWalls() {
    if (wallsVAO == 0) {

        float vertices[] = { // VOORKANT, ACHTER, LINKS, RECHTS, BOVEN , ONDER
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

         0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };
        glGenVertexArrays(1, &wallsVAO);
        glGenBuffers(1, &wallsVBO);
        glBindVertexArray(wallsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, wallsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(wallsVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
void drawTegels() {
    if (tegelsVAO == 0) {
        float verticesTegels[] = {
        0.5f,  -0.5f,  0.5f,   1.0f, 1.0f, // top right
        0.5f,  -0.5f, -0.5f,   1.0f, 0.0f, // bottom right
       -0.5f,  -0.5f, -0.5f,   0.0f, 0.0f, // bottom left
       -0.5f,  -0.5f,  0.5f,   0.0f, 1.0f,  // top left
        0.5f,  -0.5f,  0.5f,   1.0f, 1.0f // top right

        };
        glGenVertexArrays(1, &tegelsVAO);
        glGenBuffers(1, &tegelsVBO);
        glBindVertexArray(tegelsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, tegelsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTegels), &verticesTegels, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(tegelsVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
    glBindVertexArray(0);
}
void renderCrosshair() {
    if (crosshairVAO == 0) {
        float verticesCrosshair[] = {
            -0.004f,  0.008f,
            -0.004f, -0.008f,
             0.004f, -0.008f,
             0.004f, -0.008f,
             0.004f,  0.008f,
            -0.004f,  0.008f


        };
        glGenVertexArrays(1, &crosshairVAO);
        glGenBuffers(1, &crosshairVBO);
        glBindVertexArray(crosshairVAO);
        glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCrosshair), &verticesCrosshair, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    }
    glBindVertexArray(crosshairVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

}
bool checkCollison(Bullet* a) {

    // Function that detects if one of the bullets hits the target. 
    // This is done by calculating the distance between bullet and target, if its less than radius of target, its a hit else its not.
    // When bullet hit, this bullet should be removed obv.

    vector< Target* >::iterator it = targets.begin();
    while (it != targets.end()) {
        float x = a->bulletPos.x - (*it)->targetPos.x;
        float y = a->bulletPos.y - (*it)->targetPos.y;
        float z = a->bulletPos.z - (*it)->targetPos.z;
        float distance = sqrt(pow(x, 2.0) + pow(y, 2) + pow(z, 2));
        if (distance <= 0.0765) {
            it = targets.erase(it);
            score++;
            return true;
        }
        else {
            ++it;
        }
    }
    return false;
}
float getPoint(float n1, float n2, float perc)
{
    float diff = n2 - n1;

    return n1 + (diff * perc);
}
void renderWalls(Shader brickShader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, std::vector<glm::vec3> cubePositions, unsigned int brickTexture) {
    // activate shader
    brickShader.use();
    brickShader.setMat4("projection", projection);
    brickShader.setMat4("view", view);

    // render boxes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickTexture);
    for (unsigned int i = 0; i < cubePositions.size(); i++)
    {
        // calculate the model matrix for each object and pass it to shader before drawing
        model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * 0;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        brickShader.setMat4("model", model);

        drawWalls();
    }
}
void renderTegels(Shader tegelShader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, std::vector<glm::vec3> tegelsPostions, unsigned int tegelTexture) {
    // activate shader
    tegelShader.use();
    tegelShader.setMat4("projection", projection);
    tegelShader.setMat4("view", view);

    // render boxes
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tegelTexture);
    for (unsigned int i = 0; i < tegelsPostions.size(); i++)
    {
        // calculate the model matrix for each object and pass it to shader before drawing
        model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        model = glm::translate(model, tegelsPostions[i]);
        float angle = 20.0f * 0;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        tegelShader.setMat4("model", model);

        drawTegels();
    }
}
void renderGun(Shader weaponArmShader, glm::mat4 model, glm::mat4 view, glm::mat4 projection, Model gunModel) {
    model = glm::mat4(1.0f);
    yawWeapon = -(cam.getYaw() - 90.0f);
    pitchWeapon = cam.getPitch();
    updateGunPosForRotate();

    weaponArmShader.use();
    weaponArmShader.setMat4("projection", projection);
    weaponArmShader.setMat4("view", view);

    model = glm::translate(model, glm::vec3(gunPos));
    model = glm::rotate(model, glm::radians(yawWeapon), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(pitchWeapon), glm::vec3(1.0f, 0.0f, 0.0f)); // Model rotatie X op model, maar Z in rotatiematrix (lang gezocht voor dit)

    weaponArmShader.setMat4("model", model);
    gunModel.Draw(weaponArmShader);
}
void renderBullets(Shader bulletShader, Model bulletModel, glm::mat4 view, glm::mat4 projection) {
    int size = bullets.size();
    for (int i = 0; i < size; i++) {
        bulletShader.use();
        bulletShader.setMat4("projection", projection);
        bulletShader.setMat4("view", view);
        bullets[i]->draw();
        bulletShader.setMat4("model", bullets[i]->model);
        bulletModel.Draw(bulletShader);
        bullets[i]->updatePos();
        if (checkCollison(bullets[i]))
        {
            bullets.erase(bullets.begin() + i);
            engine->play2D("resources/sound/hit.wav");
        }
        if (size != (int)bullets.size())
        {
            --i;
            size = bullets.size();
        }

    }
}
void renderTargets(Shader targetShader, Model targetModel, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
    for (int i = 0; i < targets.size(); i++) {
        model = glm::mat4(1.0f);
        glBindTexture(GL_TEXTURE_2D, 0);
        targetShader.use();
        targetShader.setMat4("projection", projection);
        targetShader.setMat4("view", view);

        model = glm::translate(model, targets[i]->targetPos);

        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
        targetShader.setMat4("model", model);
        targetModel.Draw(targetShader);

        targets[i]->bezier();

    }
}
void renderFences(Shader fenceShader, Model fenceModel, glm::mat4 model, glm::mat4 view, glm::mat4 projection, std::vector<glm::vec4> fencePos) {
    for (int i = 0; i < fencePos.size(); i++) {
        model = glm::mat4(1.0f);
        glBindTexture(GL_TEXTURE_2D, 0);
        fenceShader.use();
        fenceShader.setMat4("projection", projection);
        fenceShader.setMat4("view", view);

        model = glm::translate(model, glm::vec3(fencePos[i]));

        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        fenceShader.setMat4("model", model);
        fenceModel.Draw(fenceShader);
    }
}
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
void generateWallVertices(std::vector<glm::vec3>& vec)
{
    for (int i = 0; i < 11; i++) { vec.push_back(glm::vec3((float)i, 0.0f, 0.0f)); vec.push_back(glm::vec3((float)i, 0.0f, 7.0f)); }
    for (int i = 1; i < 7; i++) { vec.push_back(glm::vec3(10.0f, 0.0f, (float)i)); vec.push_back(glm::vec3(0.0f, 0.0f, (float)i)); }
}
void generateTegelsVertices(std::vector<glm::vec3>& vec)
{
    for (int i = 1; i < 10; i++) {
        for (int j = 1; j < 7; j++) {
            vec.push_back(glm::vec3((float)i, 0.0f, (float)j));
        }
    }
}
void generateFencePos(std::vector<glm::vec4>& vec) {
    float start = 6.42;
    for (int i = 0; i < 5; i++) {
        vec.push_back(glm::vec4(7.05f, -0.55f, start - i*1.17f, 0.0f));
    }
}
void updateGunPosForRotate() {
    //maak rotationmatrix
    glm::mat4 trans = glm::mat4(1.0f);
    // translatie punt waarond we gaan roteren naar origin == gunoffsett dus niet meer nodig 

    // doe rotatie
    trans = glm::rotate(trans, glm::radians(yawWeapon + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    trans = glm::rotate(trans, glm::radians(pitchWeapon), glm::vec3(0.0f, 0.0f, 1.0f)); // Bij rotatie x op model = rotatie z op matrix
    glm::vec4 result = trans * gunOffsetFromCam;
    // translation terug naar punt waaron we geroteerd hebben en sla deze pos in gunpos.
    gunPos.x = result.x + cam.playerPosition.x;
    gunPos.y = result.y + cam.playerPosition.y;
    gunPos.z = result.z + cam.playerPosition.z;
}
void renderText(Shader shader, std::string text) {
    unsigned int textVAO, textVBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCREEN_WIDTH), 0.0f, static_cast<float>(SCREEN_HEIGHT));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    RenderText(shader, text, 25.0f, 25.0f, 1.0f, glm::vec3(1, 1, 1), textVAO, textVBO);
}
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int textVAO, unsigned int textVBO)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale; 
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        engine->play2D("resources/sound/gunshot.mp3");
        bullets.push_back(new Bullet(yawWeapon, pitchWeapon, glm::vec4(cam.playerPosition, 0.0f)));
    }
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.processKeys(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.processKeys(BACKWARDS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.processKeys(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.processKeys(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        wireframeOn = !wireframeOn;
        if (wireframeOn) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double posX, double posY)
{
    if (firstMouseMove)
    {
        lastX = (float)posX;
        lastY = (float)posY;
        firstMouseMove = false;
    }

    float offsetX = (float)posX - lastX;
    float offsetY = lastY - (float)posY;
    lastX = (float)posX;
    lastY = (float)posY;
    cam.processMouse(offsetX, offsetY);
}