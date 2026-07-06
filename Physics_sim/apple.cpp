#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//Shaders
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "out vec3 FragPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "out vec3 Normal;\n" 
    "uniform mat4 projection;\n"   
    "void main()\n"
    "{\n"
    "FragPos = vec3(model * vec4(aPos, 1.0));\n"               // set first
    "Normal = mat3(transpose(inverse(model))) * aNormal;\n"
    "gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 FragPos;\n"
    "in vec3 Normal;\n"
    "uniform vec3 lightPos;\n"
    "uniform vec3 lightColor;\n"
    "uniform vec3 objectColor;\n"
    "void main()\n"
    "{\n"
    "float ambientStrength = 0.4;\n"
    "vec3 ambient = ambientStrength * lightColor;\n"
    "vec3 norm     = normalize(Normal);\n"
        "vec3 lightDir = normalize(lightPos - FragPos);\n"
        "float diff    = max(dot(norm, lightDir), 0.0);\n"
        "vec3 diffuse  = diff * lightColor;\n"
        "FragColor = vec4((ambient + diffuse) * objectColor, 1.0);\n"
    "}\n\0";
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float delTime = 0.0f;
float lastFrame = 0.0f;
float posX = 0.0f;
float posY = -1.0f;

float velY = 1.2f;

struct SubMesh {
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
    glm::vec3 color = glm::vec3(1.0f);
};

struct Model {
    std::vector<SubMesh> meshes;
};

Model loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!scene || !scene->mRootNode) {
        std::cerr << "Failed to load: " << importer.GetErrorString() << "\n";
        return {};
    }

    std::cout << "Loaded: " << path << "\n";
    std::cout << "Total meshes: " << scene->mNumMeshes << "\n";

    Model model;

    // ── loop through ALL meshes ───────────────────────────────────────────
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        SubMesh result;

        std::cout << "Mesh " << m << ": "
                  << mesh->mNumVertices << " vertices, "
                  << mesh->mNumFaces    << " faces\n";

        // material color
        glm::vec3 objectColor(1.0f, 0.0f, 0.0f); // default red
        if (scene->mNumMaterials > 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiColor3D color(0.0f, 0.0f, 0.0f);
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
                objectColor = glm::vec3(color.r, color.g, color.b);
        }
        result.color = objectColor;

        // vertices + normals
        std::vector<float> vertices;
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        }

        // indices
        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        result.indexCount = indices.size();

        // upload to GPU
        glGenVertexArrays(1, &result.VAO);
        glGenBuffers(1, &result.VBO);
        glGenBuffers(1, &result.EBO);

        glBindVertexArray(result.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, result.VBO);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
            6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        model.meshes.push_back(result);
    }

    return model;
}
int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Apple", NULL, NULL);
    glfwSetWindowPos(window, 700, 250);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);  

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    Model myModel = loadModel("C:\\OpenGL\\untitled.obj");
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glUseProgram(shaderProgram); 
    int lightPosLoc   = glGetUniformLocation(shaderProgram, "lightPos");
    int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    int objColorLoc   = glGetUniformLocation(shaderProgram, "objectColor");
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    // render loop
    // -----------
    
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
        glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float angle = (float)glfwGetTime();          // grows each frame
        glm::mat4 view;
        view = glm::lookAt(cameraPos,cameraPos+cameraFront, cameraUp);  
 
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f,100.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glm::mat4 transform = glm::mat4(1.0f);

        glm::mat4 model1 = glm::mat4(1.0f);
       


// world space bounds at Z=0
        velY+= 9.8f*delTime;
        posY+= velY *delTime;

// bounce
        float halfH       = tan(glm::radians(45.0f / 2.0f)) * 3.0f;
        float boxHalfSize = 0.3f * 0.5f;
        float boundY      = halfH - boxHalfSize;

        if (posY >  boundY || posY<-boundY) { posY =  boundY; velY = -0.75f*velY; }
        model1 = glm::translate(model1, glm::vec3(posX, -posY, 0.0f));
        model1 = glm::scale(model1, glm::vec3(0.4f, 0.4f, 0.4f));
        //model1 = glm::rotate(model1, angle, glm::vec3(0.0f, 1.0f, 0.0f));           
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glUniform3f(lightPosLoc,   2.0f, 2.0f, 2.0f);
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
        for (auto& subMesh : myModel.meshes) {
        glUniform3f(objColorLoc, subMesh.color.r,subMesh.color.g,subMesh.color.b);
        glBindVertexArray(subMesh.VAO);
         glDrawElements(GL_TRIANGLES, subMesh.indexCount, GL_UNSIGNED_INT, 0);}

/*         glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(vx+1.0f, 0.0f, vy));  
        //model2 = glm::rotate(model2, angle * glm::radians(50.0f), glm::vec3(1.0f, 2.0f, 1.0f));             
        model2 = glm::scale(model2, glm::vec3(0.3f, 0.3f, 0.3f));     
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
        //glUniform3f(colorTintLoc, 0.0f, 0.0f, 0.0f); 
        glUniform1i(uIsHoleLoc, false);  
        glDrawElements(GL_TRIANGLES, 36,GL_UNSIGNED_INT, 0); */
        
       /*  glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, glm::vec3(0.0f, 0.0f, -0.02f));  // right
        model3 = glm::rotate(model3, angle * glm::radians(50.0f), glm::vec3(1.0f, 2.0f, 1.0f));             // same spin
        //model3 = glm::scale(model3, glm::vec3(0.2f, 0.2f, 0.2f));      // half size

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
        glUniform3f(colorTintLoc, 0.2f, 0.2f, 0.2f); // red tint

        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2); */

        //glBindVertexArray(0); // no need to unbind it every time 
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

   for (auto& subMesh : myModel.meshes) {
    glDeleteVertexArrays(1, &subMesh.VAO);
    glDeleteBuffers(1, &subMesh.VBO);
    glDeleteBuffers(1, &subMesh.EBO);}

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    float currentFrame = glfwGetTime();
    delTime=currentFrame-lastFrame;
    lastFrame = currentFrame;  
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    const float cameraSpeed = 2.5f  *delTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}