#ifdef _WIN32
  #include <windows.h>
#endif

// OpenGL & GLEW (GL Extension Wrangler)
#include <GL/glew.h>
// GLFW
#include <glfw3.h>

// GLM for matrix transformation
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

// GLSL Wrangler
#include <glsw/glsw.h>

// Standard libraries
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>

// imgui
#include <tools/imgui.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>

#include <tools/stb_image.h>
#include <tools/SimpleProfile.h>

#include <tools/TCamera.hpp>
#include <tools/Timer.hpp>
#include <tools/Logger.hpp>
#include <tools/gltools.hpp>

#include <GLType/ProgramShader.h>
#include <GLType/BaseTexture.h>
#include <SkyBox.h>
#include <Mesh.h>
#include <ModelAssImp.h>

#include <fstream>
#include <memory>
#include <vector>
#include <algorithm>

namespace
{
	struct fRGB {
		float r, g, b; 
	};
    const unsigned int WINDOW_WIDTH = 1280;
    const unsigned int WINDOW_HEIGHT = 720;
	const char* WINDOW_NAME = "Basic Example";

	bool bCloseApp = false;
	GLFWwindow* window = nullptr;  

    //?

    TCamera camera;

    GLuint m_EmptyVAO = 0;
    bool bWireframe = false;

    //?

	void initialize(int argc, char** argv);
	void initExtension();
	void initGL();
	void initWindow(int argc, char** argv);
    void finalizeApp();
	void mainLoopApp();
    void moveCamera( int key, bool isPressed );
	void prepareRender();
    void render();
	void renderHUD();
	void update();
	void updateHUD();

	void glfw_error_callback(int error, const char* description);
    void glfw_keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void glfw_reshape_callback(GLFWwindow* window, int width, int height);
    void glfw_mouse_callback(GLFWwindow* window, int button, int action, int mods);
    void glfw_motion_callback(GLFWwindow* window, double xpos, double ypos);
	void glfw_char_callback(GLFWwindow* windows, unsigned int c);
	void glfw_scroll_callback(GLFWwindow* windows, double xoffset, double yoffset);
}

// Breakpoints that should ALWAYS trigger (EVEN IN RELEASE BUILDS) [x86]!
#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak()
#else 
#include <signal.h>
#define DEBUG_BREAK raise(SIGTRAP)
// __builtin_trap() 
#endif

void APIENTRY OpenglCallbackFunction(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    using namespace std;

    // ignore these non-significant error codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
        return;

    cout << "---------------------opengl-callback-start------------" << endl;
    cout << "message: " << message << endl;
    cout << "type: ";
    switch(type) {
    case GL_DEBUG_TYPE_ERROR:
        cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        cout << "OTHER";
        break;
    }
    cout << endl;

    cout << "id: " << id << endl;
    cout << "severity: ";
    switch(severity){
    case GL_DEBUG_SEVERITY_LOW:
        cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        cout << "HIGH";
        break;
    }
    cout << endl;
    cout << "---------------------opengl-callback-end--------------" << endl;
    if (type == GL_DEBUG_TYPE_ERROR)
        DEBUG_BREAK;
}

namespace {
	using namespace std;

	typedef std::vector<std::istream::char_type> ByteArray;
	ByteArray ReadFileSync( const std::string& name )
	{
		std::ifstream inputFile;
		inputFile.open( name, std::ios::binary | std::ios::ate );
		if (!inputFile.is_open())
			return ByteArray();
		auto filesize = static_cast<size_t>(inputFile.tellg());
		ByteArray buf = ByteArray(filesize * sizeof( char ) );
		inputFile.ignore( std::numeric_limits<std::streamsize>::max() );
		inputFile.seekg( std::ios::beg );
		inputFile.read( reinterpret_cast<char*>(buf.data()), filesize );
		inputFile.close();
		return buf;
	}

    glm::mat4 envViewMtx()
    {
        glm::vec3 toTargetNorm = camera.getDirection();
        glm::vec3 fakeUp { 0, 1, 0 };
        glm::vec3 right = glm::cross(fakeUp, toTargetNorm);
        glm::vec3 up = glm::cross(toTargetNorm, right);
        return glm::mat4( 
            glm::vec4(right, 0), 
            glm::vec4(up, 0),
            glm::vec4(toTargetNorm, 0),
            glm::vec4(0, 0, 0, 1));
    }

	void initialize(int argc, char** argv)
	{
		// window maanger
		initWindow(argc, argv);

		// OpenGL extensions
		initExtension();

		// OpenGL
		initGL();

		// ImGui initialize without callbacks
		ImGui_ImplGlfwGL3_Init(window, false);

		// Load FontsR
		// (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
		// ImGuiIO& io = ImGui::GetIO();
		// io.Fonts->AddFontDefault();
		// io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
		// io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
		// io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
		// io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);

		// GLSW : shader file manager
		glswInit();
		glswSetPath("./shaders/", ".glsl");
		glswAddDirectiveToken("*", "#version 330 core");

        // App Objects
        camera.setViewParams( glm::vec3( 5.0f, 5.0f, 20.0f), glm::vec3( 5.0f, 5.0f, 0.0f) );
        camera.setMoveCoefficient(0.35f);

        Timer::getInstance().start();

        GLuint m_VertexArrayID;
        GL_ASSERT(glGenVertexArrays(1, &m_VertexArrayID));
        GL_ASSERT(glBindVertexArray(m_VertexArrayID));

		// to prevent osx input bug
		fflush(stdout);

		prepareRender();
	}

	void initExtension()
	{
        glewExperimental = GL_TRUE;

        GLenum result = glewInit(); 
        if (result != GLEW_OK)
        {
            fprintf( stderr, "Error: %s\n", glewGetErrorString(result));
            exit( EXIT_FAILURE );
        }

        fprintf( stderr, "GLEW version : %s\n", glewGetString(GLEW_VERSION));
	}

	void initGL()
	{
        // Clear the error buffer (it starts with an error).
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err << std::endl;
		}

        std::printf("%s\n%s\n", 
			glGetString(GL_RENDERER),  // e.g. Intel HD Graphics 3000 OpenGL Engine
			glGetString(GL_VERSION)    // e.g. 3.2 INTEL-8.0.61
        );

    #if _DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        if (glDebugMessageCallback) {
            cout << "Register OpenGL debug callback " << endl;
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
            glDebugMessageCallback(OpenglCallbackFunction, nullptr);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
        else
            cout << "glDebugMessageCallback not available" << endl;
    #endif

        glClearColor( 0.15f, 0.15f, 0.15f, 0.0f);

        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LEQUAL );

        glDisable( GL_STENCIL_TEST );
        glClearStencil( 0 );

        glCullFace( GL_BACK );    
        glFrontFace(GL_CCW);

        glDisable( GL_MULTISAMPLE );

        glGenVertexArrays(1, &m_EmptyVAO);
	}

	void initWindow(int argc, char** argv)
	{
		glfwSetErrorCallback(glfw_error_callback);

		// Initialise GLFW
		if( !glfwInit() )
		{
			fprintf( stderr, "Failed to initialize GLFW\n" );
			exit( EXIT_FAILURE );
		}
        
        glfwWindowHint(GLFW_SAMPLES, 4);
    #ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL );
		if ( window == NULL ) {
			fprintf( stderr, "Failed to open GLFW window\n" );
			glfwTerminate();
			exit( EXIT_FAILURE );
		}
		glfwMakeContextCurrent(window);
		glfw_reshape_callback( window, WINDOW_WIDTH, WINDOW_HEIGHT );

		// GLFW Events' Callback
		glfwSetWindowSizeCallback( window, glfw_reshape_callback );
		glfwSetKeyCallback( window, glfw_keyboard_callback );
		glfwSetMouseButtonCallback( window, glfw_mouse_callback );
		glfwSetCursorPosCallback( window, glfw_motion_callback );
		glfwSetCharCallback( window, glfw_char_callback );
		glfwSetScrollCallback( window, glfw_scroll_callback );
	}

	void finalizeApp()
	{
        glswShutdown();  
        Logger::getInstance().close();
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
	}

	void mainLoopApp()
	{
		do {
			update();
			render();

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}
		while (!bCloseApp && glfwWindowShouldClose(window) == 0);
	}

    // GLFW Callbacks_________________________________________________  
	void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

    void glfw_reshape_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);

        float aspectRatio = ((float)width) / ((float)height);
        camera.setProjectionParams( 45.0f, aspectRatio, 0.1f, 250.0f);
    }

	void update()
	{
        Timer::getInstance().update();
        camera.update();
        // app.update();
		updateHUD();
	}

	void updateHUD()
	{
		ImGui_ImplGlfwGL3_NewFrame();
	}

	void prepareRender()
	{
	}

    void render()
    {    
        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 0);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );    
        glPolygonMode(GL_FRONT_AND_BACK, (bWireframe)? GL_LINE : GL_FILL);
		renderHUD();
    }

	void renderHUD()
	{
		// restore some state
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        ImGui::Render();
	}

    void glfw_keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
	{
		ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);

		moveCamera( key, action != GLFW_RELEASE);
		bool bPressed = action == GLFW_PRESS;
		if (bPressed) {
			switch (key)
			{
				case GLFW_KEY_ESCAPE:
					bCloseApp = true;

				case GLFW_KEY_W:
					bWireframe = !bWireframe;
					break;

				case GLFW_KEY_T:
					{
						Timer &timer = Timer::getInstance();
						printf( "fps : %d [%.3f ms]\n", timer.getFPS(), timer.getElapsedTime());
					}
					break;

				case GLFW_KEY_R:
					break;

				default:
					break;
			}
		}
    }

    void moveCamera( int key, bool isPressed )
    {
        switch (key)
        {
		case GLFW_KEY_UP:
			camera.keyboardHandler( MOVE_FORWARD, isPressed);
			break;

		case GLFW_KEY_DOWN:
			camera.keyboardHandler( MOVE_BACKWARD, isPressed);
			break;

		case GLFW_KEY_LEFT:
			camera.keyboardHandler( MOVE_LEFT, isPressed);
			break;

		case GLFW_KEY_RIGHT:
			camera.keyboardHandler( MOVE_RIGHT, isPressed);
			break;
        }
    }

    void glfw_motion_callback(GLFWwindow* window, double xpos, double ypos)
    {
		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		const bool bPressed = (state == GLFW_PRESS);
		const bool mouseOverGui = ImGui::MouseOverArea();
		if (!mouseOverGui && bPressed) camera.motionHandler( int(xpos), int(ypos), false);    
    }  

    void glfw_mouse_callback(GLFWwindow* window, int button, int action, int mods)
    {
        ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) { 
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			const bool mouseOverGui = ImGui::MouseOverArea();
			if (!mouseOverGui)
				camera.motionHandler( xpos, ypos, true); 
		}    
    }


	void glfw_char_callback(GLFWwindow* windows, unsigned int c)
	{
		ImGui_ImplGlfwGL3_CharCallback(windows, c);
	}

	void glfw_scroll_callback(GLFWwindow* windows, double xoffset, double yoffset)
	{
		ImGui_ImplGlfwGL3_ScrollCallback(windows, xoffset, yoffset);
	}
}

int main(int argc, char** argv)
{
	initialize(argc, argv);
	mainLoopApp();
	finalizeApp();
    return EXIT_SUCCESS;
}
