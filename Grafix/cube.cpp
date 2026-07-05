#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//Shaders
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "out vec3 ourNormal;\n" 
    "uniform mat4 projection;\n"   
    "uniform vec3 uColorTint;\n"
    "void main()\n"
    "{\n"
    "gl_Position = projection * view * model * vec4(aPos, 1.0);\n"  
    "ourNormal = mat3(transpose(inverse(model))) * vec3(0.0, 0.0, 1.0);\n"
    "ourColor = aColor * uColorTint;\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "in vec3 ourNormal;\n"
    "uniform bool uIsHole;\n"
    "void main()\n"
    "{\n"
    "   if (uIsHole)\n"                                          
    "       FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
      
    "   else\n"                                          
    "       FragColor = vec4(ourColor, 1.0f);\n"
    "}\n\0";

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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cubus", NULL, NULL);
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

    
    float vertices[] = {
    // positions         // colors
    0.5f,0.5f,0.5f,  0.0f, 0.0f, 1.0f,   //1
    -0.5f,0.5f,0.5f,  1.0f, 0.0f, 1.0f,  //2
    -0.5f,-0.5f,0.5f,  0.0f, 1.0f, 1.0f, //3
    0.5f,-0.5f,0.5f,  1.0f, 1.0f, 1.0f,  //4
    
    0.5f,0.5f,-0.5f,  0.0f, 0.0f, 1.0f,  //5
    -0.5f,0.5f,-0.5f,  1.0f, 0.0f, 1.0f, //6
   -0.5f,-0.5f,-0.5f,  0.0f, 1.0f, 1.0f, //7
    0.5f,-0.5f,-0.5f,  1.0f, 1.0f, 1.0f  //8
};    
unsigned int indices[] = {

    0, 1, 2,   2, 3, 0,  

    4, 5, 6,   6, 7, 4,

    4, 0, 3,   3, 7, 4,

    1, 5, 6,   6, 2, 1,

    4, 5, 1,   1, 0, 4,

    3, 2, 6,   6, 7, 3
};

    unsigned int VBO, VAO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
   
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
        vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
        indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnableVertexAttribArray(1); 

    glUseProgram(shaderProgram); 
    
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    int colorTintLoc = glGetUniformLocation(shaderProgram, "uColorTint");
    int uIsHoleLoc = glGetUniformLocation(shaderProgram, "uIsHole");
    
    // render loop

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // render

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float angle = (float)glfwGetTime();          // grows each frame
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); 
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
       
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glm::mat4 transform = glm::mat4(1.0f);

        
        float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
        glBindVertexArray(VAO);

        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model1 = glm::rotate(model1, angle * glm::radians(50.0f), glm::vec3(1.0f, 2.0f, 1.0f));           
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glUniform3f(colorTintLoc, 1.0f, 1.0f, 1.0f); 
        glUniform1i(uIsHoleLoc, false);  
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, 0.1f));  
        model2 = glm::rotate(model2, angle * glm::radians(50.0f), glm::vec3(1.0f, 2.0f, 1.0f));             
        model2 = glm::scale(model2, glm::vec3(0.3f, 0.3f, 0.3f));     
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
        glUniform3f(colorTintLoc, 0.0f, 0.0f, 0.0f); 
        glUniform1i(uIsHoleLoc, true);  
        glDrawElements(GL_TRIANGLES, 36,GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
