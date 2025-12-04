#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include "shader_s.h"
#include "stb_image.h"

#include<iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 730;
const unsigned int SCR_HEIGHT = 1200;

int effectMode = 5;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    Shader originalShader("fullscreen.vs", "original.fs");
    Shader blurShader("fullscreen.vs", "blur.fs");
    Shader vignetteShader("fullscreen.vs", "vignette.fs");
    Shader polaroidShader("fullscreen.vs", "polaroid.fs");
   
    float vertices[] = {
        //positions   //texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f

    }; 

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texcoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create IMAGE texture 
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); 
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    unsigned char* data = stbi_load("testimage.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //Intialize ping pong
    unsigned int pingFBO[2];
    unsigned int pingTex[2];
    glGenFramebuffers(2, pingFBO);
    glGenTextures(2, pingTex);

    //Setup ping pong buffers
    for (int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingFBO[i]);

        glBindTexture(GL_TEXTURE_2D, pingTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingTex[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // MODE 1: ORIGINAL IMAGE
        if (effectMode == 1)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            originalShader.use();
            glBindTexture(GL_TEXTURE_2D, texture);

            // draw
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }


        // PASS 1: Copy original → pingTex[0]
        glBindFramebuffer(GL_FRAMEBUFFER, pingFBO[0]);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        glClear(GL_COLOR_BUFFER_BIT);
        originalShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // MODE 2: Add BLUR 
        if (effectMode == 2)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT);

            blurShader.use();
            glBindTexture(GL_TEXTURE_2D, pingTex[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }


        // PASS 2: Add Blur ping[0] → ping[1]
        glBindFramebuffer(GL_FRAMEBUFFER, pingFBO[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        blurShader.use();
        glBindTexture(GL_TEXTURE_2D, pingTex[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // MODE 3: Add vignette
        if (effectMode == 3)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT);

            vignetteShader.use();
            glBindTexture(GL_TEXTURE_2D, pingTex[1]);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }


        // PASS 3: Add Vignette ping[1] → ping[0]
        glBindFramebuffer(GL_FRAMEBUFFER, pingFBO[0]);
        glClear(GL_COLOR_BUFFER_BIT);
        vignetteShader.use();
        glBindTexture(GL_TEXTURE_2D, pingTex[1]);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // MODE 4: FULL PIPELINE
        // polaroid(ping[0]) → screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);

        polaroidShader.use();
        glBindTexture(GL_TEXTURE_2D, pingTex[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteFramebuffers(2, pingFBO);
    glDeleteTextures(2, pingTex);
    glDeleteTextures(1, &texture);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) effectMode = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) effectMode = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) effectMode = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) effectMode = 4;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) effectMode = 5;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}