//========================================================================================================================================================================================================================
// DEMO 052 : Ocean rendering
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "log.hpp"
#include "constants.hpp"
#include "gl_info.hpp"
#include "glfw_window.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "shader.hpp"
#include "fft.hpp"

struct demo_window_t : public glfw_window_t
{
    camera_t camera;

    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : glfw_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen /*, true */),
          camera(70.5, 0.125)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);
        camera.infinite_perspective(constants::two_pi / 6.0f, aspect(), 0.1f);
    }

    //===================================================================================================================================================================================================================
    // mouse handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
        if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(frame_dt);
        else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(frame_dt);
        else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(frame_dt);
        else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(frame_dt);

        if ((key == GLFW_KEY_KP_ADD) && (action == GLFW_RELEASE))
		    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if ((key == GLFW_KEY_KP_SUBTRACT) && (action == GLFW_RELEASE))
		    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void on_mouse_move() override
    {
        double norm = glm::length(mouse_delta);
        if (norm > 0.01)
            camera.rotateXY(mouse_delta / norm, norm * frame_dt);
    }
};


struct ocean_vertex_t
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};	


struct ocean_t
{
	int N, Nplus1, half_N;																		// dimension -- N should be a power of 2

	float spectral_factor;																		// phillips spectrum parameter -- affects heights of waves
	glm::vec2 wind;																				// wind direction
    float W;                                                                                    // wind velocity
	float length;																				// length parameter
	const float damping = 0.001;
													
	complex_t<float> *h_tilde0, *h_tilde1, *h_tilde, *h_tilde_slopex, *h_tilde_slopey, *h_tilde_dx, *h_tilde_dy;		// for fast fourier transform

	ocean_vertex_t *vertices;																	// vertices for vertex buffer object
	unsigned int *indices;																		// indicies for vertex buffer object
	unsigned int index_count;																	// number of indices to render

	GLuint vao_id;																				// vertex array object
	GLuint vbo_id, ibo_id;																		// vertex and index buffer objects

	ocean_t(const int N, const float spectral_factor, const glm::vec2& wind, const float length);
	~ocean_t();
	void release();         
	float dispersion(int n, int m);																// deep water
	float phillips(int n, int m);																// phillips spectrum
	complex_t<float> hTilde_0(int n, int m);
	complex_t<float> hTilde(float t, int n, int m);
	void evaluate_fft(float t);
    void render(uniform_t& uniform_model_matrix, float t);
};

ocean_t::ocean_t(const int N, const float spectral_factor, const glm::vec2& w, const float length) :
	N(N), spectral_factor(spectral_factor), length(length)
{
    W = glm::length(w);                                                                         // separate w into wind magnitude ...
    wind = w / W;                                                                               // ... and direction
 
	Nplus1 = N + 1;
	half_N = N / 2;
	unsigned int sqr = N * N;

	h_tilde0       = (complex_t<float>*) malloc(7 * sqr * sizeof(complex_t<float>));
	h_tilde1       = h_tilde0 + sqr;
	h_tilde        = h_tilde1 + sqr;
	h_tilde_slopex = h_tilde + sqr;
	h_tilde_slopey = h_tilde_slopex + sqr;
	h_tilde_dx     = h_tilde_slopey + sqr;
	h_tilde_dy     = h_tilde_dx + sqr;

	vertices = (ocean_vertex_t*) malloc(Nplus1 * Nplus1 * sizeof(ocean_vertex_t));
	indices = (unsigned int*) malloc(6 * sqr * sizeof(unsigned int));

	for (int m = 0; m < Nplus1; ++m)
	{
		for (int n = 0; n < Nplus1; ++n)
		{
			ocean_vertex_t& vertex = vertices[m * Nplus1 + n];
			vertex.uv = (length / N) * glm::vec2(n - half_N, m - half_N);
			vertex.position = glm::vec3(vertex.uv, 0.0f);
			vertex.normal = glm::vec3(0.0, 0.0, 1.0);
		}
	}

	index_count = 0;
	for (int m = 0; m < N; m++)
	{
		for (int n = 0; n < N; n++)
		{
            int i0 = m * N + n;

			h_tilde0[i0] = hTilde_0( n,  m);
			h_tilde1[i0] = 0.0;//hTilde_0(-n,  -m); // hTilde_0(-n, -m).conj();

			int i1 = i0 + m;
            indices[index_count++] = i1;             
            indices[index_count++] = i1 + 1;         
            indices[index_count++] = i1 + Nplus1 + 1;
            indices[index_count++] = i1;             
            indices[index_count++] = i1 + Nplus1 + 1;
            indices[index_count++] = i1 + Nplus1;    
		}
	}

	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ocean_vertex_t) * Nplus1 * Nplus1, vertices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ocean_vertex_t), (const GLvoid *) offsetof(ocean_vertex_t, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ocean_vertex_t), (const GLvoid *) offsetof(ocean_vertex_t, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ocean_vertex_t), (const GLvoid *) offsetof(ocean_vertex_t, uv));
}

ocean_t::~ocean_t() 
{
	free(h_tilde0);
	free(vertices);
	free(indices);
}

void ocean_t::release()
{
	glDeleteBuffers(1, &vbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
}

float ocean_t::dispersion(int n, int m)
{
	glm::vec2 k = glm::vec2(n - half_N, m - half_N);
	return glm::sqrt(constants::two_pi * constants::g * glm::length(k) / length);
}

float ocean_t::phillips(int n, int m)
{
    int n1 = n - half_N;
    int m1 = m - half_N;
    if (!n1 && !m1) return 0.0;

	glm::vec2 k = (constants::two_pi / length) * glm::vec2(n1, m1);
    float l = glm::length(k);
	float l2 = l * l;
	float l4 = l2 * l2;
	float cos_theta = glm::dot(k, wind) / l;
	float cos2 = cos_theta * cos_theta;
	float cos4 = cos2 * cos2;
	float L = W * W / constants::g;
	float L2 = L * L;
	float Q = l2 * L2 * damping * damping;							
	return spectral_factor * exp(-1.0f / (l2 * L2)) / l4 * cos4 * exp(-Q);
}


std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<float> gauss_rand(0.0f, 1.0f);

complex_t<float> ocean_t::hTilde_0(int n, int m)
{
	complex_t<float> r(gauss_rand(gen), gauss_rand(gen));
	return r * sqrt(phillips(n, m) / 2.0f);
}

complex_t<float> ocean_t::hTilde(float t, int n, int m)
{
	int i = m * N + n;
	float omega = dispersion(n, m) * t;
	float cs = cos(omega);
	float sn = sin(omega);

    complex_t<float> h0 = h_tilde0[i];
    complex_t<float> h1 = h_tilde1[i];

	return (h0 * complex_t<float>(cs,  sn)) + h1 * complex_t<float>(cs, -sn);
}

void ocean_t::evaluate_fft(float t)
{
    //===================================================================================================================================================================================================================
    // update fourier coefficients
    //===================================================================================================================================================================================================================
	int exception = half_N * Nplus1;
	h_tilde_dx[exception] = 0.0f;
	h_tilde_dy[exception] = 0.0f;

	for (int m = 0; m < N; m++)
	{
		glm::vec2 k;
		k.y = constants::two_pi * (m - half_N) / length;
		for (int n = 0; n < N; n++)
		{
			k.x = constants::two_pi * (n - half_N) / length;
			float l = glm::length(k);
			int i = m * N + n;

			h_tilde[i] = hTilde(t, n, m);

			h_tilde_slopex[i] = complex_t<float>(-h_tilde[i].im * k.x, h_tilde[i].re * k.x);
			h_tilde_slopey[i] = complex_t<float>(-h_tilde[i].im * k.y, h_tilde[i].re * k.y);

			if (i != exception)
			{
				h_tilde_dx[i] = complex_t<float>(h_tilde[i].im * k.x / l, -h_tilde[i].re * k.x / l);
				h_tilde_dy[i] = complex_t<float>(h_tilde[i].im * k.y / l, -h_tilde[i].re * k.y / l);
			}
		}
	}

	unsigned int size = 8; 

    fft::transform2d<float, true>(h_tilde, size, size);
    fft::transform2d<float, true>(h_tilde_slopex, size, size);
    fft::transform2d<float, true>(h_tilde_slopey, size, size);
    fft::transform2d<float, true>(h_tilde_dx, size, size);
    fft::transform2d<float, true>(h_tilde_dy, size, size);

	for (int m = 0; m < N; m++)
	{
		for (int n = 0; n < N; n++)
		{
			int i0 = m * N + n;														// index into h_tilde..
			int i1 = m * Nplus1 + n;												// index into vertices

			if ((m + n) & 1)
			{
				h_tilde[i0] = -h_tilde[i0];
				h_tilde_dx[i0] = -h_tilde_dx[i0];										// displacement
				h_tilde_dy[i0] = -h_tilde_dy[i0];
				h_tilde_slopex[i0] = -h_tilde_slopex[i0];								// normal
				h_tilde_slopey[i0] = -h_tilde_slopey[i0];

			}
			vertices[i1].position = glm::vec3(vertices[i1].uv.x - h_tilde_dx[i0].re, vertices[i1].uv.y - h_tilde_dy[i0].re, h_tilde[i0].re);
			vertices[i1].normal = glm::normalize(glm::vec3(-h_tilde_slopex[i0].re, -h_tilde_slopey[i0].re, 1.0f));
		}
	}

    for(int k = 0; k < N; ++k)
    {
		int i0 = k * N;
		int i1 = i0 + k;
		vertices[i1 + N].position = glm::vec3(vertices[i1 + N].uv.x - h_tilde_dx[i0].re, vertices[i1 + N].uv.y - h_tilde_dy[i0].re, h_tilde[i0].re);
		vertices[i1 + N].normal = vertices[i1].normal;
		vertices[k + Nplus1 * N].position = glm::vec3(vertices[k + Nplus1 * N].uv.x - h_tilde_dx[k].re, vertices[k + Nplus1 * N].uv.y - h_tilde_dy[k].re, h_tilde[k].re);
		vertices[k + Nplus1 * N].normal = vertices[k].normal;
    }
	vertices[Nplus1 * Nplus1 - 1].position = glm::vec3(vertices[N + Nplus1 * N].uv.x - h_tilde_dx[0].re, vertices[N + Nplus1 * N].uv.y - h_tilde_dy[0].re, h_tilde[0].re);
	vertices[Nplus1 * Nplus1 - 1].normal = vertices[0].normal;
}

void ocean_t::render(uniform_t& uniform_model_matrix, float t)
{
	evaluate_fft(t);

	glBindVertexArray(vao_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ocean_vertex_t) * Nplus1 * Nplus1, vertices);

	for (int j = 0; j < 8; j++)
	{
		for (int i = 0; i < 2; i++)
		{
            float scale_factor = 75.0;
			glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));
			model_matrix = glm::translate(model_matrix, glm::vec3(length * i, -j * length, 0.0f));
			uniform_model_matrix = model_matrix;
        	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
		}
	}
}

int main(int argc, char *argv[])
{
    //===================================================================================================================================================================================================================
    // initialize GLFW library
    // create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("Ocean using FFT", 4, 3, 3, 1920, 1080, true);

	ocean_t ocean(256, 0.00031f, glm::vec2(0.0f, 16.0f), 48.0);										// ocean simulator

    //===================================================================================================================================================================================================================
    // phong lighting model shader initialization
    //===================================================================================================================================================================================================================
    glsl_program_t ocean_shader(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ocean.vs"),
                                glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/ocean.fs"));
    ocean_shader.enable();

    uniform_t uniform_projection_view_matrix = ocean_shader["projection_view_matrix"];
    uniform_t uniform_model_matrix           = ocean_shader["model_matrix"];
    uniform_t uniform_camera_ws              = ocean_shader["camera_ws"];
    uniform_t uniform_time              	 = ocean_shader["time"];
	
	ocean_shader["water_texture"] = 0;

    glActiveTexture(GL_TEXTURE0);
  	GLuint water_texture_id = image::png::texture2d("../../../resources/tex2d/water4.png");

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
	double start_time = glfw::time();
	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    while(!window.should_close())
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);										// rendering
        window.new_frame();
        float time = window.frame_ts;
        
		uniform_projection_view_matrix = window.camera.projection_view_matrix();
		uniform_camera_ws = window.camera.position();
		uniform_time = time;

		ocean.render(uniform_model_matrix, time);
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
	ocean.release();
    glfw::terminate();
    return 0;
}