#include "GL/glew.h"
#include <GL/freeglut.h>
#include <math.h>
#include "SOIL/SOIL.h"
#include "stdio.h"

//som
#include<windows.h>
#include<MMsystem.h>

struct astro {
    GLuint id;                          //Id da display list com os vértices do astro

    float angZ;                         //Angulo de rotação em volta do eixo z (eixo de rotação do sol, translação para os planetas)
    float incrementoTranslacao;         //Incremento do angulo quando o astro gira em torno do sol. No caso do sol, esse é incremento do seu angulo em torno do proprio eixo

    float angRot;                       //Angulo de rotação em torno do próprio eixo

    //Coordenadas do centro do astro, levando em conta as transformações já feitas
    float x;
    float y;
    float z;

    float raio;                         //Raio do astro

    GLuint idTextura;                   //Id da textura do astro
};

char basepath[1000];

struct astro sol;

struct astro mercurio;
struct astro venus;
struct astro terra;
    struct astro lua;
struct astro marte;
    struct astro lua2;
struct astro jupiter;
struct astro saturno;
struct astro urano;
struct astro netuno;
struct astro plutao;

int posCamera;                  //Id da posição atual da câmera
int camy;                       //coordenada y da posição da câmera
int camz;                       //coordenada z da posição da câmera
BOOLEAN iluminacao;             //TRUE: se a iluminação estiver ligada
BOOLEAN orbitasVisiveis;        //TRUE: se as órbitas dos planetas estão visíveis


//Cores
float branco[] = {1.0, 1.0, 1.0};

//Constantes de luz
const GLfloat luz_ambiente[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat luz_difusa[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat luz_especular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat luz_posicao[] = { 10.0f, 10.0f, 10.0f, 0.0f};

// Propriedades do material dos astros
float ambienteDifusaMaterial[] = {1.0, 1.0, 1.0, 1.0};    // cor ambiente e difusa: branca
float especularMaterial[] = { 1.0, 1.0, 1,0, 1.0 };       // cor especular: branca
float expEspecular[] = {50.0};
float compEmissivaSol[] = {1.0, 0.2, 0.0, -1.0};
float compEmissivaPlanetas[] = {0.0, 0.0, 0.0, -1.0};

// Função que escreve o texto em s na tela
void escreveNaTela(char *s, float x, float y) {

    glRasterPos3f(x, y, -100);

    for (int i = 0; i < (int)strlen(s); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, s[i]);
    }
}

// Função que carrega uma textura e retorna a sua id
GLuint carregaTextura(const char* arquivo) {
    char caminhoArquivo[1000];
    strcpy(caminhoArquivo, basepath);
    strcat(caminhoArquivo, "imagens\\");
    strcat(caminhoArquivo, arquivo);

    GLuint idTextura = SOIL_load_OGL_texture(caminhoArquivo, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if (idTextura == 0)
        printf("Erro arquivo %s SOIL: '%s'\n", arquivo, SOIL_last_result());
    return idTextura;
}

//Função que toca a música de fundo
void playBackgroundSound(char * arquivoSom){
    //path relativo
    char caminhoArquivo[1000];
    strcpy(caminhoArquivo, basepath);
    strcat(caminhoArquivo, "musica\\");
    strcat(caminhoArquivo, arquivoSom);
    PlaySound(TEXT(caminhoArquivo),NULL,SND_LOOP|SND_ASYNC);
}

//Inicializa os valores das variáveis do astro passado como parâmetro, além de criar uma display list contendo os vértices
void inicializaAstro(struct astro *umAstro, GLuint idAstroDisplayList, float incrementoTranslacao, float x, float z,
                     float raio, const char* arquivoTextura, BOOLEAN temAnel) {

    //Inicialização das variáveis
    umAstro->id = idAstroDisplayList;
    umAstro->angZ = 0.0;
    umAstro->incrementoTranslacao = incrementoTranslacao;
    umAstro->angRot=0.0;
    umAstro->x = x;
    umAstro->y = 0.0;
    umAstro->z = z;
    umAstro->raio = raio;
    umAstro->idTextura = carregaTextura(arquivoTextura);

    //Geração da displaylist
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
    glNewList(umAstro->id, GL_COMPILE);
        glEnable(GL_TEXTURE_2D);

            glFrontFace(GL_CCW);        //Define o sentido antihorário como sendo o sentido da face da frente
            glCullFace(GL_BACK);        //Corta a face de trás

            //Configura a textura
            glBindTexture(GL_TEXTURE_2D, umAstro->idTextura );
            GLUquadric *qobj = gluNewQuadric();
            gluQuadricTexture(qobj, GL_TRUE);

            //Desenha o sólido e, se temAnel = TRUE, o anel
            gluSphere(qobj, umAstro->raio, 30, 50);
            if(temAnel) {
                glRotated(20.0, 0.0, 1.0, 0);
                glutSolidTorus(umAstro->raio * 0.2, umAstro->raio +1 , 30, 50);
            }
            gluDeleteQuadric(qobj);

        glDisable(GL_TEXTURE_2D);
    glEndList();

}

//Desenha o astro a partir da displaylist
void desenhaAstro(struct astro *umAstro){
    glRotated(umAstro->angZ, 0.0, 0.0, 1.0);        //Rotação em torno do do eixo do sol
    glTranslated(umAstro->x, umAstro->y, umAstro->z);
    glPushMatrix();
        glRotated(umAstro->angRot, 0.5, 0, 0);      //Rotação em torno do proprio eixo
    glPopMatrix();
    glCallList(umAstro->id);
    umAstro->angZ+=umAstro->incrementoTranslacao/2;
    umAstro->angRot+=0.1;
}


//Função, responsável por iterar as coordenadas da câmera para dar o efeito de trransição quando as teclas 1, 2 ou 3 são pressionadas
        //A iteração pára quando a câmera atinge a posição desejada (posicaoFinal)
int iteraPosicaoCamera (int posicaoAtual, int posicaoFinal) {
    if(posicaoAtual > posicaoFinal) {
        posicaoAtual--;
        Sleep(15);
    } else if(posicaoAtual < posicaoFinal){
        posicaoAtual++;
        Sleep(15);
    }
    return posicaoAtual;
}

//Função que posiciona a camera de acordo com a posição definida em posicaoCamera
void posicionaCamera (int posicaoCamera) {
    switch (posicaoCamera) {
    case 1:             //Vista de cima

        camy = iteraPosicaoCamera(camy, 0);
        camz = iteraPosicaoCamera(camz, 0);

        //Posição final/desejada (0, 0, 0, ...)
        gluLookAt(0,camy, camz,         //Posição da camera
                  0,0,-100,             //Vetor que aponta para a cena
                  0,1,0);               //Vetor up
        break;

    case 2:            //Vista angular

        camy = iteraPosicaoCamera(camy, -100);
        camz = iteraPosicaoCamera(camz, 0);


        //Posição final/desejada (0, -100, 0, ...)
        gluLookAt(0,camy,camz,    0,0,-100,     0,1,0);
        break;

    case 3:             //Vista lateral

        camy = iteraPosicaoCamera(camy, -100);
        camz = iteraPosicaoCamera(camz, -99);


        //Posição final/desejada (0, -100, -99, ...)
        gluLookAt(0,camy,camz,    0,0,-100,     0,1,0);
        break;
    }
    glutPostRedisplay();
}


// Rotina de desenho
void desenha()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();


    //Escreve na tela
    glDisable(GL_LIGHTING);
    glColor3f(branco[0], branco[1], branco[2]);

    escreveNaTela((char*)"Pressione:", -60, 40);
    escreveNaTela((char*)"1 - Vista de cima", -55, 35);
    escreveNaTela((char*)"2 - Vista angular", -55, 30);
    escreveNaTela((char*)"3 - Vista lateral", -55, 25);
    escreveNaTela((char*)"4 - Liga/Desliga a iluminacao", -55, 20);
    escreveNaTela((char*)"5 - Deixa as orbitas visiveis/invisiveis", -55, 15);

    glEnable(GL_LIGHTING);

    //Configurações de Iluminaçãp
    glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	//Configuração da luz do sol
    glLightfv(GL_LIGHT0, GL_AMBIENT,  luz_ambiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  luz_difusa);
	glLightfv(GL_LIGHT0, GL_SPECULAR, luz_especular);

    glLoadIdentity();

    //Posicionamento da câmera
    posicionaCamera(posCamera);

    //Desenha as órbitas dos planetas
    glDisable(GL_LIGHTING);
    glColor3f(0.5,0.5,0.5);         //Cor cinza
    if(orbitasVisiveis) {
        int lados = 20, aneis = 80;
        glPushMatrix();
            glTranslated(0,0,-100);
            glutSolidTorus(0.1, mercurio.x, lados, aneis);
            glutSolidTorus(0.1, venus.x, lados, aneis);
            glutSolidTorus(0.1, terra.x, lados, aneis);
            glutSolidTorus(0.1, marte.x, lados, aneis);
            glutSolidTorus(0.1, jupiter.x, lados, aneis);
            glutSolidTorus(0.1, saturno.x, lados, aneis);
            glutSolidTorus(0.1, urano.x, lados, aneis);
            glutSolidTorus(0.1, netuno.x, lados, aneis);
            glutSolidTorus(0.1, plutao.x, lados, aneis);
        glPopMatrix();
    }

    //Habilita a iluminação
    if(iluminacao) {
        glEnable(GL_LIGHTING);
        glColor3f(branco[0], branco[1], branco[2]);
    }

    //Desenha os astros, aplicando as transformações geométricas quando necessário
    glPushMatrix();     //Salva as configurações do modelview
        float posicaoSol[] = {sol.x, sol.y, sol.z, 1};
        glLightfv(GL_LIGHT0, GL_POSITION, posicaoSol);

        glMaterialfv(GL_FRONT, GL_EMISSION, compEmissivaSol);
        desenhaAstro(&sol);

        glMaterialfv(GL_FRONT, GL_EMISSION, compEmissivaPlanetas);
        glPushMatrix();
            desenhaAstro(&mercurio);
        glPopMatrix();
        glPushMatrix();
            desenhaAstro(&venus);
        glPopMatrix();
        glPushMatrix();

                desenhaAstro(&terra);
                desenhaAstro(&lua);
        glPopMatrix();
        glPushMatrix();
            desenhaAstro(&marte);
                desenhaAstro(&lua2);
        glPopMatrix();
        glPushMatrix();
            desenhaAstro(&jupiter);
        glPopMatrix();
        glPushMatrix();
            desenhaAstro(&saturno);
        glPopMatrix();
        glPushMatrix();
            desenhaAstro(&urano);
        glPopMatrix();
        glPushMatrix();
            desenhaAstro(&netuno);
        glPopMatrix();
        glPushMatrix();
            desenhaAstro(&plutao);
        glPopMatrix();

    glPopMatrix();      //Carrega as configurações salvas no push matrix


    glutSwapBuffers();
    glutPostRedisplay();

}

//Função que atualiza a tela
void atualiza(int x) {
    glutTimerFunc(x, atualiza, 0);
}

// NOVIDADE: uma função que vamos chamar dentro do "main"
// Inicia algumas variáveis de estado do OpenGL
void setup()
{
    // define qual é a cor do fundo
    glClearColor(0.0, 0.0, 0.0, 0.0); // preto
    glEnable(GL_DEPTH_TEST);

    // Definindo as propriedades do material
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambienteDifusaMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, especularMaterial);
    glMaterialfv(GL_FRONT, GL_SHININESS, expEspecular);

    //Toca a musica de fundo
    playBackgroundSound((char*)"Interestellar2.wav");

    //Gera as ids das displaylists
    GLuint idSol = glGenLists(10);
    GLuint idLuaTerra = glGenLists(1);
    GLuint idLuaMarte = glGenLists(1);

    //Os valores de x inicial de cada astro foi definido de modo que se possa alterar o raio de um astro sem alterar a distância entre eles
    inicializaAstro(&sol, idSol, 0.1, 0, -100.0, 5.0, "sol.jpg", FALSE);
    inicializaAstro(&mercurio, idSol+1, 0.03, sol.x+sol.raio+2+0.5, 0.0, 0.5, "mercurio.png", FALSE);
    inicializaAstro(&venus, idSol+2, 0.06, mercurio.x+mercurio.raio+2+1, 0, 0.8, "venus.jpg", FALSE);
    inicializaAstro(&terra, idSol+3, 0.09, venus.x+venus.raio+2+1.2, 0, 1.2, "terra.jpg", FALSE);
        inicializaAstro(&lua,idLuaTerra  , 0.5, 2.0+0.4, 0, 0.4, "lua.png", FALSE);
    inicializaAstro(&marte, idSol+4, 0.12, terra.x+terra.raio+2+0.8, 0, 1.0, "marte.jpg", FALSE);
        inicializaAstro(&lua2 ,idLuaMarte, 0.5, 2.0+0.4, 0, 0.7, "lua.png", FALSE);
    inicializaAstro(&jupiter, idSol+5, 0.15, marte.x+marte.raio+2+2, 0, 2.0, "jupiter.jpg", TRUE);
    inicializaAstro(&saturno, idSol+6, 0.18, jupiter.x+jupiter.raio+2+1.6, 0, 1.6, "saturno.jpg", TRUE);
    inicializaAstro(&urano, idSol+7, 0.21, saturno.x+saturno.raio+2+1.0, 0, 0.7, "urano.jpg", TRUE);
    inicializaAstro(&netuno, idSol+8, 0.24, urano.x+urano.raio+2+1.0, 0, 0.9, "netuno.jpg", TRUE);
    inicializaAstro(&plutao, idSol+9, 0.27, netuno.x+netuno.raio+2+0.5, 0, 0.5, "plutao.jpg", FALSE);

    //Inicializa algumas variáveis
    posCamera = 1;
    camy = 0;
    camz = 0;
    iluminacao = TRUE;
    orbitasVisiveis = TRUE;

}

//Função responsável pelo redimensionamento da tela, para definir a  perspectiva e manter a razão de aspecto
void redimensionada(int width, int height)
{
   // left, bottom, right, top
   glViewport(0, 0, width, height);

   //Mantém a razão de aspecto
   float aspect = (float) width / (float) height;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(50.0, aspect, 0.1, 500);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

//Callback de Teclado
void teclaPressionada(unsigned char key, int x, int y)
{
    // vê qual tecla foi pressionada
    switch(key)
    {
    case '1':
        posCamera = 1;

        break;
    case '2':
        posCamera = 2;
        break;
    case '3':
        posCamera = 3;
        break;
     case '4':
        iluminacao = !iluminacao;
        break;
    case '5':
        orbitasVisiveis = !orbitasVisiveis;
        break;

    case 27:      // Tecla "ESC"
        exit(0);  // Sai da aplicação
        break;
    default:
        break;
    }
}

// Função principal
int main(int argc, char** argv)
{
    strcpy(basepath, argv[0]);
    for(int i=strlen(basepath);((basepath[i-1]!='\\') && (i>0));i--){ basepath[i-1]='\x0'; }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Sistema Solar");

    // Registra callbacks para eventos
    glutDisplayFunc(desenha);
    glutReshapeFunc(redimensionada);
    glutKeyboardFunc(teclaPressionada);
    glutTimerFunc(33, atualiza, 0);

    // Configura valor inicial de algumas
    // variáveis de estado do OpenGL
    setup();

    glutMainLoop();
    return 0;
}
