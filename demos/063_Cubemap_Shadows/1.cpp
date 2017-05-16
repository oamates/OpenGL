// Example Depth Cube Map Shadowing
 
#include <glut/glut.h>
#include <math.h>
#include <stdlib.h>
 
 
#define WINDOW_SIZE     (512)
#define WINDOW_NEAR     (1.0)
#define WINDOW_FAR      (100.0)
 
#define SHADOW_SIZE     (256)
#define SHADOW_NEAR     (1.0)
#define SHADOW_FAR      (10.0)
 
 
static float camera_position[3] = { -8.0f, 8.0f, -8.0f };
static float camera_view_matrix[16];
static float camera_view_matrix_inv[16];
static float camera_projection_matrix[16];
 
static float light_distance = 5.0f;
static float light_inclination = 45.0f * (M_PI / 180.0f);
static float light_azimuth = 0.0f;
 
static float light_position_ws[3];
static float light_position_cs[3];
static float light_view_matrix[16];
static float light_face_matrix[6][16];
static float light_projection_matrix[16];
 
static GLuint tex_depth_cube;
 
static GLuint program_shadow;
static GLuint shader_shadow_vs;
 
static GLuint program_render;
static GLuint shader_render_vs;
static GLuint shader_render_fs;
 
static GLuint framebuffer_shadow;
 
static const char *shader_shadow_vs_source =
	"#version 120\n"
	"#extension GL_EXT_gpu_shader4 : require\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"}\n"
;
static const char *shader_render_vs_source =
	"#version 120\n"
	"#extension GL_EXT_gpu_shader4 : require\n"
	"varying vec4 position_cs;\n"
	"varying vec3 normal_cs;\n"
	"varying vec3 color;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	position_cs = gl_ModelViewMatrix * gl_Vertex;\n"
	"	normal_cs = gl_NormalMatrix * gl_Normal;\n"
	"	color = gl_Color.rgb;\n"
	"}\n"
;
static const char *shader_render_fs_source =
	"#version 120\n"
	"#extension GL_EXT_gpu_shader4 : require\n"
	"varying vec4 position_cs;\n"
	"varying vec3 normal_cs;\n"
	"varying vec3 color;\n"
	"uniform mat4x4 camera_view_matrix_inv;\n"
	"uniform mat4x4 light_view_matrix;\n"
	"uniform mat4x4 light_projection_matrix;\n"
	"uniform samplerCubeShadow shadow;\n"
	"uniform vec3 light_position;\n"
	"void main()\n"
	"{\n"
	"	vec4 position_ls = light_view_matrix * camera_view_matrix_inv * position_cs;\n"
 
	// shadow map test
	"	vec4 abs_position = abs(position_ls);\n"
	"	float fs_z = -max(abs_position.x, max(abs_position.y, abs_position.z));\n"
	"	vec4 clip = light_projection_matrix * vec4(0.0, 0.0, fs_z, 1.0);\n"
	"	float depth = (clip.z / clip.w) * 0.5 + 0.5;\n"
	"	vec4 result = shadowCube(shadow, vec4(position_ls.xyz, depth));\n"
 
	"	vec3 lvector = light_position - position_cs.xyz;\n"
	"	float ldistance = length(lvector);\n"
	"	float lintensity = max(dot(normal_cs, normalize(lvector)), 0.0) * 10.0;\n"
	"	lintensity /= ldistance * ldistance;\n"
	"	lintensity /= lintensity + 0.5;\n"
	"	vec3 diffuse = lintensity * result.xyz * color;\n"
	"	gl_FragColor = vec4(diffuse,1);\n"
	"}\n"
;
 
static void
app_init()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
 
	// create camera matrix
	glLoadIdentity();
	gluLookAt(camera_position[0], camera_position[1], camera_position[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glGetFloatv(GL_MODELVIEW_MATRIX, camera_view_matrix);
 
	// create camera inverse matrix
	camera_view_matrix_inv[ 0] = camera_view_matrix[ 0];
	camera_view_matrix_inv[ 1] = camera_view_matrix[ 4];
	camera_view_matrix_inv[ 2] = camera_view_matrix[ 8];
	camera_view_matrix_inv[ 4] = camera_view_matrix[ 1];
	camera_view_matrix_inv[ 5] = camera_view_matrix[ 5];
	camera_view_matrix_inv[ 6] = camera_view_matrix[ 9];
	camera_view_matrix_inv[ 8] = camera_view_matrix[ 2];
	camera_view_matrix_inv[ 9] = camera_view_matrix[ 6];
	camera_view_matrix_inv[10] = camera_view_matrix[10];
	camera_view_matrix_inv[12] = camera_position[0];
	camera_view_matrix_inv[13] = camera_position[1];
	camera_view_matrix_inv[14] = camera_position[2];
	camera_view_matrix_inv[ 3] = 0.0f;
	camera_view_matrix_inv[ 7] = 0.0f;
	camera_view_matrix_inv[11] = 0.0f;
	camera_view_matrix_inv[15] = 1.0f;
 
	// create light face matrices
	glLoadIdentity(); gluLookAt(0.0, 0.0, 0.0,  1.0, 0.0, 0.0,  0.0,-1.0, 0.0); // +X
	glGetFloatv(GL_MODELVIEW_MATRIX, light_face_matrix[0]);
	glLoadIdentity(); gluLookAt(0.0, 0.0, 0.0, -1.0, 0.0, 0.0,  0.0,-1.0, 0.0); // -X
	glGetFloatv(GL_MODELVIEW_MATRIX, light_face_matrix[1]);
	glLoadIdentity(); gluLookAt(0.0, 0.0, 0.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0); // +Y
	glGetFloatv(GL_MODELVIEW_MATRIX, light_face_matrix[2]);
	glLoadIdentity(); gluLookAt(0.0, 0.0, 0.0,  0.0,-1.0, 0.0,  0.0, 0.0,-1.0); // -Y
	glGetFloatv(GL_MODELVIEW_MATRIX, light_face_matrix[3]);
	glLoadIdentity(); gluLookAt(0.0, 0.0, 0.0,  0.0, 0.0, 1.0,  0.0,-1.0, 0.0); // +Z
	glGetFloatv(GL_MODELVIEW_MATRIX, light_face_matrix[4]);
	glLoadIdentity(); gluLookAt(0.0, 0.0, 0.0,  0.0, 0.0,-1.0,  0.0,-1.0, 0.0); // -Z
	glGetFloatv(GL_MODELVIEW_MATRIX, light_face_matrix[5]);
 
	// create light projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, 1.0, SHADOW_NEAR, SHADOW_FAR);
	glGetFloatv(GL_PROJECTION_MATRIX, light_projection_matrix);
	glMatrixMode(GL_MODELVIEW);
 
	// create depth cube map
	glGenTextures(1, &amp;tex_depth_cube);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_depth_cube);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	for (size_t i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
 
	// create shadow shader
	shader_shadow_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader_shadow_vs, 1, &amp;shader_shadow_vs_source, NULL);
	glCompileShader(shader_shadow_vs);
 
	// create shadow program
	program_shadow = glCreateProgram();
	glAttachShader(program_shadow, shader_shadow_vs);
	glLinkProgram(program_shadow);
 
	// create render shader
	shader_render_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader_render_vs, 1, &amp;shader_render_vs_source, NULL);
	glCompileShader(shader_render_vs);
	shader_render_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader_render_fs, 1, &amp;shader_render_fs_source, NULL);
	glCompileShader(shader_render_fs);
 
	// create render program
	program_render = glCreateProgram();
	glAttachShader(program_render, shader_render_vs);
	glAttachShader(program_render, shader_render_fs);
	glLinkProgram(program_render);
 
	// create shadow framebuffer
	glGenFramebuffersEXT(1, &amp;framebuffer_shadow);
}
 
static void
draw_shadow_casters()
{
	glColor3ub(255,0,0);
	glPushMatrix();
		glTranslated(-1.0, 0.0,-1.0);
		glutSolidSphere(1.0, 20, 10);
	glPopMatrix();
 
	glColor3ub(0,255,0);
	glPushMatrix();
		glTranslated( 1.0, 0.0, 1.0);
		glutSolidCube(2.0);
	glPopMatrix();
 
	glColor3ub(0,0,255);
	glPushMatrix();
		glTranslated( 1.0, 0.0,-1.0);
		glutSolidIcosahedron();
	glPopMatrix();
 
	glColor3ub(255,0,255);
	glPushMatrix();
		glTranslated(-1.0, 0.0, 1.0);
		glutSolidOctahedron();
	glPopMatrix();
}
 
static void
draw_scene()
{
	draw_shadow_casters();
 
	glColor3ub(255, 255, 255);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS);
		glVertex3i( 10, -1,-10);
		glVertex3i(-10, -1,-10);
		glVertex3i(-10, -1, 10);
		glVertex3i( 10, -1, 10);
	glEnd();
}
 
static void
app_update()
{
	// rotate the light about the Y-axis
	light_azimuth = fmodf(light_azimuth + (0.1f * M_PI / 180.0f), 2.0f * M_PI);
 
	// update the world space light position
	light_position_ws[0] = light_distance * sinf(light_inclination) * cosf(light_azimuth);
	light_position_ws[1] = light_distance * cosf(light_inclination);
	light_position_ws[2] = light_distance * sinf(light_inclination) * sinf(light_azimuth);
 
	// create the light view matrix (construct this as you would a camera matrix)
	glLoadIdentity();
	glTranslatef(-light_position_ws[0], -light_position_ws[1], -light_position_ws[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, light_view_matrix);
 
	// transform world space light position to camera space
	light_position_cs[0] =
		camera_view_matrix[ 0] * light_position_ws[0] +
		camera_view_matrix[ 4] * light_position_ws[1] +
		camera_view_matrix[ 8] * light_position_ws[2] +
		camera_view_matrix[12];
	light_position_cs[1] =
		camera_view_matrix[ 1] * light_position_ws[0] +
		camera_view_matrix[ 5] * light_position_ws[1] +
		camera_view_matrix[ 9] * light_position_ws[2] +
		camera_view_matrix[13];
	light_position_cs[2] =
		camera_view_matrix[ 2] * light_position_ws[0] +
		camera_view_matrix[ 6] * light_position_ws[1] +
		camera_view_matrix[10] * light_position_ws[2] +
		camera_view_matrix[14];
}
 
static void
app_display()
{
	app_update();
 
	////////////////////////////////////////////////////////////////////////////
	// RENDER DEPTH CUBE MAP
 
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_shadow);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
 
	glCullFace(GL_FRONT);
 
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(light_projection_matrix);
	glMatrixMode(GL_MODELVIEW);
 
	for (size_t i = 0; i < 6; ++i) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex_depth_cube, 0);
 
		glClear(GL_DEPTH_BUFFER_BIT);
 
		glLoadMatrixf(light_face_matrix[i]);
		glMultMatrixf(light_view_matrix);
 
		glUseProgram(program_shadow);
 
		draw_shadow_casters();
	}
 
	////////////////////////////////////////////////////////////////////////////
	// RENDER SCENE
 
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glViewport(0, 0, WINDOW_SIZE, WINDOW_SIZE);
 
	glCullFace(GL_BACK);
 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.0, WINDOW_NEAR, WINDOW_FAR);
	glMatrixMode(GL_MODELVIEW);
 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
 
	glLoadMatrixf(camera_view_matrix);
 
	glUseProgram(program_render);
	glUniform1i(glGetUniformLocation(program_render, "shadow"), 0);
	glUniform3fv(glGetUniformLocation(program_render, "light_position"), 1, light_position_cs);
	glUniformMatrix4fv(glGetUniformLocation(program_render, "camera_view_matrix_inv"), 1, GL_FALSE, camera_view_matrix_inv);
	glUniformMatrix4fv(glGetUniformLocation(program_render, "light_view_matrix"), 1, GL_FALSE, light_view_matrix);
	glUniformMatrix4fv(glGetUniformLocation(program_render, "light_projection_matrix"), 1, GL_FALSE, light_projection_matrix);
 
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_depth_cube);
 
	draw_scene();
 
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDisable(GL_TEXTURE_CUBE_MAP);
 
	glUseProgram(0);
	glPushMatrix();
		glTranslatef(light_position_ws[0], light_position_ws[1], light_position_ws[2]);
		glColor3ub(255,255,0);
		glutSolidSphere(0.1, 10, 5);
	glPopMatrix();
 
	glutSwapBuffers();
}
 
static void
app_idle()
{
	glutPostRedisplay();
}
 
int
main(int argc, char *argv[])
{
	glutInit(&amp;argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_STENCIL | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
	glutInitWindowPosition(-1, -1);
	glutCreateWindow("example");
 
	glutDisplayFunc(app_display);
	glutIdleFunc(app_idle);
 
	app_init();
 
	glutMainLoop();
 
	return (0);
}