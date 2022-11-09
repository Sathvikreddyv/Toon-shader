#include <windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "InitShader.h"    //Functions for loading shaders from text files
#include "LoadMesh.h"      //Functions for creating OpenGL buffers from mesh files
#include "LoadTexture.h"   //Functions for creating OpenGL textures from image files

static const std::string vertex_shader("Vertex_vs.glsl");
static const std::string fragment_shader("Fragment_fs.glsl");
GLuint shader_program = -1;

static const std::string mesh_name = "Amago0.obj";
static const std::string texture_name = "AmagoT.bmp";

GLuint texture_id = -1; //Texture map for mesh
MeshData mesh_data;

float angle = 50.0f;
float scale = 1.0f;

float dist = 1.0f;
float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float Acolor[4] = { 0.4f, 0.0f, 0.0f, 1.0f };
float AScolor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float Dcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float DScolor[4] = { 0.7f ,0.0f, 0.0f, 1.0f };
float Scolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float SScolor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float shine = 8.0f;
bool Check = false;

float camPos[3] = { 0.0f, 0.0f, 3.0f };
float FoV = 40.0f;
float aspectRatio = 1.0f;


//For an explanation of this program's structure see https://www.glfw.org/docs/3.3/quick.html 


void draw_gui(GLFWwindow* window)
{
   //Begin ImGui Frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   int w, h;
   glfwGetFramebufferSize(window, &w, &h);
   glViewport(0, 0, w, h);
   aspectRatio = (float)w / h;

   //Draw Gui
   ImGui::Begin("toon shading");                       
      if (ImGui::Button("Quit"))                          
      {
         glfwSetWindowShouldClose(window, GLFW_TRUE);
      }    

      // lighting 
      if (ImGui::TreeNode("lighting"))
      {
          //ambient,diffuse,specular temrs
          ImGui::ColorEdit4("Ambient light color", Acolor);
          float a = glGetUniformLocation(shader_program, "la");
          glUniform4f(a, Acolor[0], Acolor[1], Acolor[2], Acolor[3]);

          ImGui::ColorEdit4("Diffuse light color", Dcolor);
          float d = glGetUniformLocation(shader_program, "ld");
          glUniform4f(d, Dcolor[0], Dcolor[1], Dcolor[2], Dcolor[3]);

          ImGui::ColorEdit4("Specular light color", Scolor);
          float s = glGetUniformLocation(shader_program, "ls");
          glUniform4f(s, Scolor[0], Scolor[1], Scolor[2], Scolor[3]);

          /*  ImGui::ColorEdit4("Object color", color);
            float c = glGetUniformLocation(shader_program, "objectColor");
            glUniform4f(c, color[0], color[1], color[2], color[3]); */

          ImGui::ColorEdit4("Ambient surface color", AScolor);
          float as = glGetUniformLocation(shader_program, "ka");
          glUniform4f(as, AScolor[0], AScolor[1], AScolor[2], AScolor[3]);

          ImGui::ColorEdit4("Diffuse surface color", DScolor);
          float ds = glGetUniformLocation(shader_program, "kd");
          glUniform4f(ds, DScolor[0], DScolor[1], DScolor[2], DScolor[3]);

          ImGui::ColorEdit4("Specular surface color", SScolor);
          float ss = glGetUniformLocation(shader_program, "ks");
          glUniform4f(ss, SScolor[0], SScolor[1], SScolor[2], SScolor[3]);

          ImGui::SliderFloat("Shininess", &shine, 0.0f, 100.0f); // shininess/ glossiness of the object
          int Alpha = glGetUniformLocation(shader_program, "alpha");
          glUniform1f(Alpha, shine);

          ImGui::TreePop();
      }

      // Use texture
      ImGui::Checkbox("Texture", &Check);
      float ch = glGetUniformLocation(shader_program, "check");
      glUniform1f(ch, Check);

      //to observe shadows  
      ImGui::SliderFloat("Source distance", &dist, 0.0f, 2.0f); // distance of the light from the object
      int d_loc = glGetUniformLocation(shader_program, "di");
      glUniform1f(d_loc, dist);
      ImGui::SliderFloat("angle", &angle, 0.0f, 360.0f); // to rotate the object
      ImGui::SliderFloat("Scale", &scale, -10.0f, +10.0f); // to scale the object 

      // Camera movements
      ImGui::SliderFloat3("Camera Position", camPos, -5, 5);
      ImGui::SliderFloat("Field Of View: ", &FoV, 18.0f, 90.0f);

   ImGui::End();
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// This function gets called every time the scene gets redisplayed
void display(GLFWwindow* window)
{
   //Clear the screen to the color previously specified in the glClearColor(...) call.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   glm::mat4 M = glm::rotate(angle, glm::vec3(0.0f, 10.0f, 0.0f))*glm::scale(glm::vec3(scale*mesh_data.mScaleFactor));
   glm::mat4 V = glm::lookAt(glm::vec3(camPos[0], camPos[1], camPos[2]), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //for camera postion 
   glm::mat4 P = glm::perspective(FoV, aspectRatio, 0.1f, 100.0f);

   glUseProgram(shader_program);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture_id);
   int tex_loc = glGetUniformLocation(shader_program, "diffuse_tex");
   if (tex_loc != -1)
   {
      glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
   }

   //Get location for shader uniform variable
   int PVM_loc = glGetUniformLocation(shader_program, "PVM");
   if(PVM_loc != -1)
   {
      glm::mat4 PVM = P*V*M;
      //Set the value of the variable at a specific location
      glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
   }

   int M_loc = glGetUniformLocation(shader_program, "M");
   if (M_loc != -1)
   {
       glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
   }

   glBindVertexArray(mesh_data.mVao);
   glDrawElements(GL_TRIANGLES, mesh_data.mSubmesh[0].mNumIndices, GL_UNSIGNED_INT, 0);
   //For meshes with multiple submeshes use mesh_data.DrawMesh(); 

   draw_gui(window);

   /* Swap front and back buffers */
   glfwSwapBuffers(window);
}

void idle()
{
   float time_sec = static_cast<float>(glfwGetTime());

   //Pass time_sec value to the shaders
   int time_loc = glGetUniformLocation(shader_program, "time");
   if (time_loc != -1)
   {
      glUniform1f(time_loc, time_sec);
   }}

void reload_shader()
{
   GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());

   if (new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f); //change clear color if shader can't be compiled
   }
   else
   {
      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

      if (shader_program != -1)
      {
         glDeleteProgram(shader_program);
      }
      shader_program = new_shader;
   }
}

//This function gets called when a key is pressed
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   std::cout << "key : " << key << ", " << char(key) << ", scancode: " << scancode << ", action: " << action << ", mods: " << mods << std::endl;

   if(action == GLFW_PRESS)
   {
      switch(key)
      {
         case 'r':
         case 'R':
            reload_shader();     
         break;

         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
         break;     
      }
   }
}

//This function gets called when the mouse moves over the window.
void mouse_cursor(GLFWwindow* window, double x, double y)
{
    //std::cout << "cursor pos: " << x << ", " << y << std::endl;
}

//This function gets called when a mouse button is pressed.
void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    //std::cout << "button : "<< button << ", action: " << action << ", mods: " << mods << std::endl;
}

//Initialize OpenGL state. This function only gets called once.
void initOpenGL()
{
   glewInit();

   //Print out information about the OpenGL version supported by the graphics driver.	
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
   glEnable(GL_DEPTH_TEST);

   reload_shader();
   mesh_data = LoadMesh(mesh_name);
   texture_id = LoadTexture(texture_name);
}


int main(void)
{
   GLFWwindow* window;

   /* Initialize the library */
   if (!glfwInit())
   {
      return -1;
   }

   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(1024, 1024, "Final_project", NULL, NULL);
   if (!window)
   {
      glfwTerminate();
      return -1;
   }

   //Register callback functions with glfw. 
   glfwSetKeyCallback(window, keyboard);
   glfwSetCursorPosCallback(window, mouse_cursor);
   glfwSetMouseButtonCallback(window, mouse_button);

   /* Make the window's context current */
   glfwMakeContextCurrent(window);

   initOpenGL();
   
   //Init ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 150");

   /* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window))
   {
      idle();
      display(window);

      /* Poll for and process events */
      glfwPollEvents();
   }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

   glfwTerminate();
   return 0;
}