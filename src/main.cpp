#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

struct Engine {
    const int SCR_WIDTH =  800;
    const int SCR_HEIGHT = 600;

    GLFWwindow * window;

    // uniform locations
    GLint u_objColorLoc;
    GLint u_modelTransLoc;
    GLint u_viewTransLoc;
    GLint u_projTransLoc;

    Engine() {
        // === GLFW/GLAD INIT ===

        if (!glfwInit()) {
            cerr << "GLFW: window init failed" << endl;
            exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Rigid Body Simulator with Portals", NULL, NULL);
        if (!window) {
            cerr << "GLFW: window creation failed" << endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);

        // callbacks here

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            cerr << "GLAD: init failed" << endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }

    GLuint createShaderProg() {
        glEnable(GL_DEPTH_TEST);

        // vertex shader
        const char * shaderVertexSrc = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            uniform mat4 u_modelTransform;
            uniform mat4 u_viewTransform;
            uniform mat4 u_projTransform;
            void main() {
                gl_Position = u_projTransform * u_viewTransform * u_modelTransform * vec4(aPos, 1.0);
            }
        )";
        GLuint shaderVertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shaderVertex, 1, &shaderVertexSrc, nullptr);
        glCompileShader(shaderVertex);

        // fragment shader
        const char * shaderFragmentSrc = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec3 u_objColor;
            void main() {
                FragColor = vec4(u_objColor, 1.0f);
            }
        )";
        GLuint shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shaderFragment, 1, &shaderFragmentSrc, nullptr);
        glCompileShader(shaderFragment);

        // create shader program
        GLuint shaderProg = glCreateProgram();
        glAttachShader(shaderProg, shaderVertex);
        glAttachShader(shaderProg, shaderFragment);
        glLinkProgram(shaderProg);

        glDeleteShader(shaderVertex);
        glDeleteShader(shaderFragment);

        // store location of uniforms
        u_objColorLoc = glGetUniformLocation(shaderProg, "u_objColor");
        u_modelTransLoc = glGetUniformLocation(shaderProg, "u_modelTransform");
        u_viewTransLoc = glGetUniformLocation(shaderProg, "u_viewTransform");
        u_projTransLoc = glGetUniformLocation(shaderProg, "u_projTransform");

        return shaderProg;
    }

    GLuint setupGeom(vector<GLfloat> vertices, vector<GLuint> indices) {
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // 2 position floats
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return VAO;
    }

    void renderObject(GLuint VAO, GLenum mode, vector<GLuint>& indices, vec3 color, mat4 model, mat4 view, mat4 proj) {
        glBindVertexArray(VAO);

        glUniform3f(u_objColorLoc, color.r, color.g, color.b);
        glUniformMatrix4fv(u_modelTransLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(u_viewTransLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(u_projTransLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);
    }
};

int main() {
    Engine eng;
    const float width = (float) eng.SCR_WIDTH;
    const float height = (float) eng.SCR_HEIGHT;
    GLuint shader = eng.createShaderProg();

    float dt = 0.01f;

    // room geometry
    vector<GLfloat> vertsRoom = {
        -15.0f,  11.0f,
        -15.0f,   2.0f,
        -10.0f,  -2.0f,
        -10.0f, -11.0f,
         15.0f, -11.0f,
         15.0f,  11.0f
    };
    vector<GLuint> indsRoom = {
        0, 1,
        1, 2,
        2, 3,
        3, 4,
        4, 5,
        5, 0
    };
    GLuint VAO_Room = eng.setupGeom(vertsRoom, indsRoom);
    vec3 color_Room = vec3(0.67f, 0.25f, 0.0f);

    // local -> NDC transformations (local -> global handled in loop)
    mat4 view = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, -15.0f));
    mat4 projection = glm::ortho(-16.0f, 16.0f, -12.0f, 12.0f, 0.1f, 100.0f);

    // wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    float timePrev = 0.0f;
    mat4 model;
    while (!glfwWindowShouldClose(eng.window)) {
        // glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update dt
        float timeValue = (float)glfwGetTime();
        if (timeValue - timePrev > dt) {
            // sim.update();
            timePrev = timeValue;
        }

        glUseProgram(shader);

        eng.renderObject(VAO_Room, GL_LINES, indsRoom, color_Room, mat4(1.0f), view, projection);

        glfwSwapBuffers(eng.window);
        glfwPollEvents();
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteVertexArrays(1, &VAO_Room);

    glfwDestroyWindow(eng.window);
    glfwTerminate();
}
