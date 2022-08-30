#include "graphic.h"
#include "tree.h"


// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTriangles() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<const char*, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;


int main(){

    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 800 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 800, "INF01047 - 334132 - Clayton Barcelos", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowSize(window, 800, 800); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_0X/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //       |
    //       o-- ...
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Criamos um programa de GPU utilizando os shaders carregados acima
    GLuint program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Construímos a representação de um triângulo
    GLuint vertex_array_object_id = BuildTriangles();

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl".
    GLint model_uniform           = glGetUniformLocation(program_id, "model"); // Variável da matriz "model"
    GLint view_uniform            = glGetUniformLocation(program_id, "view"); // Variável da matriz "view" em shader_vertex.glsl
    GLint projection_uniform      = glGetUniformLocation(program_id, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    GLint render_as_black_uniform = glGetUniformLocation(program_id, "render_as_black"); // Variável booleana em shader_vertex.glsl

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Variáveis auxiliares utilizadas para chamada à função
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    // Ficamos em loop, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(program_id);

        // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
        // vértices apontados pelo VAO criado pela função BuildTriangles(). Veja
        // comentários detalhados dentro da definição de BuildTriangles().
        glBindVertexArray(vertex_array_object_id);

        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
        // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
        glm::vec4 camera_lookat_l    = glm::vec4(0.0f,0.0f,0.0f,1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
        glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
        glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -10.0f; // Posição do "far plane"

        if (g_UsePerspectiveProjection)
        {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
        else
        {
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 1.5f*g_CameraDistance/2.5f;
            float b = -t;
            float r = t*g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        // ##### TAREFAS DO LABORATÓRIO 3

        // Cada cópia do cubo possui uma matriz de modelagem independente,
        // já que cada cópia estará em uma posição (rotação, escala, ...)
        // diferente em relação ao espaço global (World Coordinates). Veja
        // slides 2-14 e 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        //
        // Entretanto, neste laboratório as matrizes de modelagem dos cubos
        // serão construídas de maneira hierárquica, tal que operações em
        // alguns objetos influenciem outros objetos. Por exemplo: ao
        // transladar o torso, a cabeça deve se movimentar junto.
        // Veja slides 243-273 do documento Aula_08_Sistemas_de_Coordenadas.pdf
        //
        glm::mat4 model = Matrix_Identity(); // Transformação inicial = identidade.


        // ORDEM PARA APLICAR 
        // -> Definir translacao da model
        // -> Definir escalamento
        // -> push matriz


        // Translação inicial do torso
        model = model * Matrix_Translate(g_TorsoPositionX - 1.0f, g_TorsoPositionY + 1.0f, 0.0f);
        // Guardamos matriz model atual na pilha
        PushMatrix(model);

            model = model * Matrix_Scale(0.8f, 1.0f, 0.2f);
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            DrawCube(render_as_black_uniform); // #### TORSO
            PushMatrix(model);
            // CABECA
            
                model = model * Matrix_Translate(0.0f, 0.05f, 0.0f); 
                model = model * Matrix_Scale(0.4f, -0.4f, 0.9f); // Atualizamos matriz model (multiplicação à direita) com um escalamento do braço direito
                model = model 
                      * Matrix_Rotate_Z(-g_AngleZ)  
                      * Matrix_Rotate_Y(g_AngleY)  
                      * Matrix_Rotate_X(g_AngleX); 
                PushMatrix(model);
                

                glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model)); // Enviamos matriz model atual para a GPU
                DrawCube(render_as_black_uniform); 
                PopMatrix(model);
            PopMatrix(model);

        // Tiramos da pilha a matriz model guardada anteriormente
        PopMatrix(model);

        // BRACO DIREITO
        PushMatrix(model); // Guardamos matriz model atual na pilha
            model = model * Matrix_Translate(-0.55f, 0.0f, 0.0f); // Atualizamos matriz model (multiplicação à direita) com uma translação para o braço direito
            //PushMatrix(model); // Guardamos matriz model atual na pilha
                model = model // Atualizamos matriz model (multiplicação à direita) com a rotação do braço direito
                      * Matrix_Rotate_Z(g_AngleZ)  // TERCEIRO rotação Z de Euler
                      * Matrix_Rotate_Y(g_AngleY)  // SEGUNDO rotação Y de Euler
                      * Matrix_Rotate_X(g_AngleX); // PRIMEIRO rotação X de Euler
                
                PushMatrix(model); // Guardamos matriz model atual na pilha
                    model = model * Matrix_Scale(0.2f, 0.6f, 0.2f); // Atualizamos matriz model (multiplicação à direita) com um escalamento do braço direito
                    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model)); // Enviamos matriz model atual para a GPU
                    DrawCube(render_as_black_uniform); // #### BRAÇO DIREITO // Desenhamos o braço direito
                PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
                
                
                PushMatrix(model); // Guardamos matriz model atual na pilha
                    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f); // Atualizamos matriz model (multiplicação à direita) com a translação do antebraço direito
                    model = model // Atualizamos matriz model (multiplicação à direita) com a rotação do antebraço direito
                          * Matrix_Rotate_Z(g_ForearmAngleZ)  // SEGUNDO rotação Z de Euler
                          * Matrix_Rotate_X(g_ForearmAngleX); // PRIMEIRO rotação X de Euler
                    PushMatrix(model); // Guardamos matriz model atual na pilha
                        model = model * Matrix_Scale(0.2f, 0.6f, 0.2f); // Atualizamos matriz model (multiplicação à direita) com um escalamento do antebraço direito
                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model)); // Enviamos matriz model atual para a GPU
                        DrawCube(render_as_black_uniform); // #### ANTEBRAÇO DIREITO // Desenhamos o antebraço direito
                    PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
                    
                    PushMatrix(model); // Guardamos matriz model atual na pilha
                    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f); // Atualizamos matriz model (multiplicação à direita) com a translação do antebraço direito
                    model = model // Atualizamos matriz model (multiplicação à direita) com a rotação do antebraço direito
                          * Matrix_Rotate_Z(g_ForearmAngleZ)  // SEGUNDO rotação Z de Euler
                          * Matrix_Rotate_X(g_ForearmAngleX); // PRIMEIRO rotação X de Euler
                    PushMatrix(model); // Guardamos matriz model atual na pilha
                        model = model * Matrix_Scale(0.2f, 0.2f, 0.2f); // Atualizamos matriz model (multiplicação à direita) com um escalamento do antebraço direito
                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model)); // Enviamos matriz model atual para a GPU
                        DrawCube(render_as_black_uniform); // #### ANTEBRAÇO DIREITO // Desenhamos o antebraço direito
                    
                    PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
                PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
            //PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
        PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
        PopMatrix(model);

        // BRACO ESQUERDO
        PushMatrix(model); // Guardamos matriz model atual na pilha
            model = model * Matrix_Translate(0.55f, 0.0f, 0.0f); // Atualizamos matriz model (multiplicação à direita) com uma translação para o braço direito
            PushMatrix(model); // Guardamos matriz model atual na pilha
                model = model // Atualizamos matriz model (multiplicação à direita) com a rotação do braço direito
                      * Matrix_Rotate_Z(-g_AngleZ)  // TERCEIRO rotação Z de Euler
                      * Matrix_Rotate_Y(g_AngleY)  // SEGUNDO rotação Y de Euler
                      * Matrix_Rotate_X(g_AngleX); // PRIMEIRO rotação X de Euler
                
                PushMatrix(model); // Guardamos matriz model atual na pilha
                    model = model * Matrix_Scale(0.2f, 0.6f, 0.2f); // Atualizamos matriz model (multiplicação à direita) com um escalamento do braço direito
                    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model)); // Enviamos matriz model atual para a GPU
                    DrawCube(render_as_black_uniform); // #### BRAÇO DIREITO // Desenhamos o braço direito
                PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
                
                
                PushMatrix(model); // Guardamos matriz model atual na pilha
                    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f); // Atualizamos matriz model (multiplicação à direita) com a translação do antebraço direito
                    model = model // Atualizamos matriz model (multiplicação à direita) com a rotação do antebraço direito
                          * Matrix_Rotate_Z(-g_ForearmAngleZ)  // SEGUNDO rotação Z de Euler
                          * Matrix_Rotate_X(g_ForearmAngleX); // PRIMEIRO rotação X de Euler
                    PushMatrix(model); // Guardamos matriz model atual na pilha
                        model = model * Matrix_Scale(0.2f, 0.6f, 0.2f); // Atualizamos matriz model (multiplicação à direita) com um escalamento do antebraço direito
                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model)); // Enviamos matriz model atual para a GPU
                        DrawCube(render_as_black_uniform); // #### ANTEBRAÇO DIREITO // Desenhamos o antebraço direito
                    PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
                    
                    PushMatrix(model); // Guardamos matriz model atual na pilha
                    model = model * Matrix_Translate(0.0f, -0.65f, 0.0f); // Atualizamos matriz model (multiplicação à direita) com a translação do antebraço direito
                    model = model // Atualizamos matriz model (multiplicação à direita) com a rotação do antebraço direito
                          * Matrix_Rotate_Z(-g_ForearmAngleZ)  // SEGUNDO rotação Z de Euler
                          * Matrix_Rotate_X(g_ForearmAngleX); // PRIMEIRO rotação X de Euler
                    PushMatrix(model); // Guardamos matriz model atual na pilha
                        model = model * Matrix_Scale(0.2f, 0.2f, 0.2f); // Atualizamos matriz model (multiplicação à direita) com um escalamento do antebraço direito
                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model)); // Enviamos matriz model atual para a GPU
                        DrawCube(render_as_black_uniform); // #### ANTEBRAÇO DIREITO // Desenhamos o antebraço direito
                    PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
                PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
            PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
        PopMatrix(model); // Tiramos da pilha a matriz model guardada anteriormente
        PopMatrix(model);

        // MEMBROS INFERIORES
        PushMatrix(model);
            model = model * Matrix_Translate(-0.2f, -1.05f, 0.0f);
            PushMatrix(model);
                model = model * Matrix_Scale(0.3f, 0.8f, 0.2f);
                glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                DrawCube(render_as_black_uniform);

                model = model * Matrix_Translate(0.0f, -1.05f, 0.0f);
                PushMatrix(model);
                    model = model * Matrix_Scale(0.8f, 0.8f, 1.0f);
                    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                    DrawCube(render_as_black_uniform);

                    model = model * Matrix_Translate(0.0f, -1.05f, 0.7f);
                    PushMatrix(model);
                        model = model * Matrix_Scale(0.8f, 0.2f, 3.0f);
                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        DrawCube(render_as_black_uniform);

                    PopMatrix(model);

                PopMatrix(model);
                
            PopMatrix(model);

        PopMatrix(model);

        PushMatrix(model);
            model = model * Matrix_Translate(0.2f, -1.05f, 0.0f);
            PushMatrix(model);
                model = model * Matrix_Scale(0.3f, 0.8f, 0.2f);
                glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                DrawCube(render_as_black_uniform);

                model = model * Matrix_Translate(0.0f, -1.05f, 0.0f);
                PushMatrix(model);
                    model = model * Matrix_Scale(0.8f, 0.8f, 1.0f);
                    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                    DrawCube(render_as_black_uniform);

                    model = model * Matrix_Translate(0.0f, -1.05f, 0.7f);
                    PushMatrix(model);
                        model = model * Matrix_Scale(0.8f, 0.2f, 3.0f);
                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        DrawCube(render_as_black_uniform);

                    PopMatrix(model);

                PopMatrix(model);
                
            PopMatrix(model);

        PopMatrix(model);




        // Neste ponto a matriz model recuperada é a matriz inicial (translação do torso)

        // Agora queremos desenhar os eixos XYZ de coordenadas GLOBAIS.
        // Para tanto, colocamos a matriz de modelagem igual à identidade.
        // Veja slides 2-14 e 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        model = Matrix_Identity();

        // Enviamos a nova matriz "model" para a placa de vídeo (GPU). Veja o
        // arquivo "shader_vertex.glsl".
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

        // Pedimos para OpenGL desenhar linhas com largura de 10 pixels.
        glLineWidth(10.0f);

        // Informamos para a placa de vídeo (GPU) que a variável booleana
        // "render_as_black" deve ser colocada como "false". Veja o arquivo
        // "shader_vertex.glsl".
        glUniform1i(render_as_black_uniform, false);

        // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
        // apontados pelo VAO como linhas. Veja a definição de
        // g_VirtualScene["axes"] dentro da função BuildTriangles(), e veja
        // a documentação da função glDrawElements() em
        // http://docs.gl/gl3/glDrawElements.
        glDrawElements(
            g_VirtualScene["axes"].rendering_mode,
            g_VirtualScene["axes"].num_indices,
            GL_UNSIGNED_INT,
            (void*)g_VirtualScene["axes"].first_index
        );

        // "Desligamos" o VAO, evitando assim que operações posteriores venham a
        // alterar o mesmo. Isso evita bugs.
        glBindVertexArray(0);

        // Pegamos um vértice com coordenadas de modelo (0.5, 0.5, 0.5, 1) e o
        // passamos por todos os sistemas de coordenadas armazenados nas
        // matrizes the_model, the_view, e the_projection; e escrevemos na tela
        // as matrizes e pontos resultantes dessas transformações.
        //glm::vec4 p_model(0.5f, 0.5f, 0.5f, 1.0f);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}
