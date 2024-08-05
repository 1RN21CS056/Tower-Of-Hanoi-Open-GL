#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Global variables for window dimensions and number of disks
GLfloat WIDTH = 1360;
GLfloat HEIGHT = 768;
GLint NUM_DISKS;

GLboolean motion = GL_FALSE;
GLfloat xangle = 0, yangle = 0;

// Macro for calculating the third peg index
#define other(i,j) (6-(i+j))
#define DISK_HEIGHT 35
#define CONE NUM_DISKS+1
#define HANOI_SOLVE 0
#define HANOI_QUIT 1

// Light colors for different lights
GLfloat lightTwoColor[] = { 1.0, 0.0, 1, 1.0 };
GLfloat lightZeroColor[] = { .3, .3, .3, .3 };

// Colors for disks and poles
GLfloat diskColor[] = { 0.0, 0.0, 1.0 };
GLfloat poleColor[] = { 1.0, 1.0, 1.0 };
GLfloat destinationPoleColor[] = { 1.0, 0.0, 0.0 }; // New color for destination peg

// Definition of a stack node and stack structure for poles
typedef struct stack_node {
    int size;
    struct stack_node* next;
} stack_node;

typedef struct stack {
    struct stack_node* head;
    int depth;
} stack;

stack poles[4];

// Function to push a disk onto a pole
int push(int which, int size) {
    stack_node* next = (stack_node*)malloc(sizeof(stack_node));
    if (!next) {
        //standard error
        fprintf(stderr, "out of memory!\n");
        exit(-1);
    }
    next->size = size;
    next->next = poles[which].head;
    poles[which].head = next;
    poles[which].depth++;
    return 0;
}

// Function to pop a disk from a pole
int pop(int which) {
    int retval = poles[which].head->size;
    stack_node* temp = poles[which].head;
    poles[which].head = poles[which].head->next;
    poles[which].depth--;
    free(temp);
    return retval;
}

// Definition of a move node and move stack structure for moves
typedef struct move_node {
    int t, f;
    struct move_node* next;
} move_node;

typedef struct move_stack {
    int depth;
    struct move_node* head, * tail;
} move_stack;

move_stack moves;

// Function to initialize OpenGL and tower of Hanoi setup
void init(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Fully dark background color
    glShadeModel(GL_SMOOTH);  // Enable smooth shading
    int i;
    for (i = 0; i < 4; i++) {
        poles[i].head = NULL;
        poles[i].depth = 0;
    }
    moves.head = NULL;
    moves.tail = NULL;
    moves.depth = 0;

    // Create display lists for disks
    for (i = 1; i <= NUM_DISKS; i++) {
        glNewList(i, GL_COMPILE);
        {
            // here we are increasing the desk shape and size
            glutSolidTorus(DISK_HEIGHT / 2 + 2, 14 * i + 2, 3, 20);  // Define disk shape
        }
        glEndList();
    }

    // Create display list for cone (representing pegs)
    glNewList(CONE, GL_COMPILE);
    {
        glutSolidCone(5, (NUM_DISKS + 2) * DISK_HEIGHT, 50, 50);  // Define cone shape
    }
    glEndList();
}

// Function to pop a move from the move stack
void mpop(void) {
    move_node* temp = moves.head;
    moves.head = moves.head->next;
    free(temp);
    moves.depth--;
}

// Function to push a move onto the move stack
void mpush(int t, int f) {
    move_node* new1 = (move_node*)malloc(sizeof(move_node));
    new1->t = t;
    new1->f = f;
    new1->next = NULL;
    if (moves.tail)
        moves.tail->next = new1;
    moves.tail = new1;
    if (!moves.head)
        moves.head = moves.tail;
    moves.depth++;
}

// Function to update the display
void update(void) {
    while (motion == GL_TRUE && motion++) {
        glutPostRedisplay();
    }
}

// Function to draw a single peg
void DrawPost(GLfloat xcenter, int pegIndex) {
    glPushMatrix();
    {
        glTranslatef(xcenter, 0, 0);
        glRotatef(90, -1, 0, 0);
        if (pegIndex == 3) {
            glColor3fv(destinationPoleColor);  // Set destination peg color
            glMaterialfv(GL_FRONT, GL_DIFFUSE, destinationPoleColor);  // Set material properties
        }
        else {
            glColor3fv(poleColor);  // Set pole color
            glMaterialfv(GL_FRONT, GL_DIFFUSE, poleColor);  // Set material properties
        }
        glCallList(CONE);
    }
    glPopMatrix();
}

// Function to draw all three pegs
void DrawPosts(void) {
    glLineWidth(10);  // Set line width for poles
    DrawPost((int)(WIDTH / 4), 1);  // Draw first peg
    DrawPost((int)(2 * WIDTH / 4), 2);  // Draw second peg
    DrawPost((int)(3 * WIDTH / 4), 3);  // Draw third peg
}

// Function to draw a single disk
void DrawDisk(GLfloat xcenter, GLfloat ycenter, GLfloat size) {
    glPushMatrix();
    {
        glTranslatef(xcenter, ycenter, 0);
        glRotatef(90, 1, 0, 0);
        glColor3fv(diskColor);  // Set disk color
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diskColor);  // Set material properties
        glCallList(size);  // Draw disk using display list
    }
    glPopMatrix();
}

// Function to draw all disks on all pegs
void DrawDisks(void) {
    int i;
    stack_node* temp;
    int xcenter, ycenter;
    for (i = 1; i <= 3; i++) {
        xcenter = i * WIDTH / 4;
        for (temp = poles[i].head, ycenter = DISK_HEIGHT * poles[i].depth - DISK_HEIGHT / 2; temp; temp = temp->next, ycenter -= DISK_HEIGHT) {
            DrawDisk(xcenter, ycenter, temp->size);  // Draw each disk on the current peg
        }
    }
}

// Macro for pushing a move onto the move stack
#define MOVE(t,f) mpush((t),(f))

// Recursive function to solve Tower of Hanoi
static void mov(int n, int f, int t) {
    int o;
    if (n == 1) {
        MOVE(t, f);  // Move disk directly
        printf("\nDisk moves From Peg: %d -> Peg: %d", f, t);  // Print move details
        return;
    }
    o = other(f, t);  // Calculate auxiliary peg
    mov(n - 1, f, o);  // Move n-1 disks to auxiliary peg
    mov(1, f, t);  // Move nth disk to target peg
    mov(n - 1, o, t);  // Move n-1 disks from auxiliary peg to target peg
}

// Function to draw the entire scene
void draw(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    {
        glTranslatef(WIDTH / 2, HEIGHT / 2, 0);
        glRotatef(xangle, 0, 1, 0);
        glRotatef(yangle, 1, 0, 0);
        glTranslatef(-WIDTH / 2, -HEIGHT / 2, 0);
        DrawPosts();  // Draw pegs
        DrawDisks();  // Draw disks
    }
    glPopMatrix();

    if (motion && moves.depth) {
        int t = moves.head->t;
        int f = moves.head->f;
        push(t, pop(f));  // Perform move operation
        mpop();  // Pop move from stack
    }
    glutSwapBuffers();
    update();  // Update display if motion is ongoing
}

// Function to handle visibility events in GLUT
void hanoi_visibility(int state) {
    if (state == GLUT_VISIBLE && motion)
        update();
}

// Variables for mouse interaction
int moving = 0;
int startx, starty;

// Function to handle mouse button events in GLUT
void hanoi_mouse(int but, int state, int x, int y) {
    if (but == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        moving = 1;
        startx = x;
        starty = y;
    }
    if (but == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        moving = 0;
    }
    if (but == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
        motion = !motion; // Toggle motion on middle button click
        update(); // Update display
    }
}

// Function to handle mouse motion events in GLUT
void hanoi_motion(int x, int y) {
    if (moving) {
        xangle += (x - startx); // Adjust rotation angles based on mouse motion
        yangle += (y - starty);
        startx = x;
        starty = y;
        glutPostRedisplay(); // Redraw the scene
    }
}

// Function to handle menu selection in GLUT
void hanoi_menu(int val) {
    switch (val) {
    case HANOI_SOLVE: motion = !motion; update(); break; // Toggle motion on solve menu item
    case HANOI_QUIT: exit(0); // Quit program on quit menu item
    }
}

// Main function
int main(int argc, char* argv[]) {
    int i;

    printf("Enter the number of Disks: ");
    scanf_s("%d", &NUM_DISKS);  // Input number of disks

    double DISK_MOVES = pow(2.0, NUM_DISKS) - 1;
    printf("\nGuide: Dr Sudhamani MJ, Associate Profssor, CSE Dept");
    printf("\nThis is CG Mini Project, Made By Eshwar k (1RN21CS056)\n");
    printf("\nTotal number of Disk movements for %d disks : %.0lf\n", NUM_DISKS, DISK_MOVES);  // Calculate and print total moves

    if (NUM_DISKS > 0) {
        glutInit(&argc, argv);
        glutInitWindowSize(1000, 600);
        glutInitWindowPosition(100, 100);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  // Set display mode
        glutCreateWindow("Tower of Hanoi");
        glutDisplayFunc(draw);  // Set draw function for window

        glViewport(0, 0, (int)WIDTH, (int)HEIGHT);  // Set viewport
        glMatrixMode(GL_PROJECTION);  // Set matrix mode
        glLoadIdentity();  // Load identity matrix
        glOrtho(0, WIDTH, -70.0, HEIGHT, -10000, 10000);  // Set orthographic projection
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClearColor(0, 1.0, 0, 0);  // Set clear color to green
        glClearDepth(1.0);
        glEnable(GL_DEPTH_TEST);

        glLightfv(GL_LIGHT2, GL_DIFFUSE, lightTwoColor);
        glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 10);
        glEnable(GL_LIGHT2);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);

        glutMouseFunc(hanoi_mouse);  // Set mouse function
        glutMotionFunc(hanoi_motion);  // Set motion function
        glutVisibilityFunc(hanoi_visibility);

        glutCreateMenu(hanoi_menu);
        glutAddMenuEntry("Solve", HANOI_SOLVE);
        glutAddMenuEntry("Quit", HANOI_QUIT);
        glutAttachMenu(GLUT_RIGHT_BUTTON);
        init();  // Initialize Tower of Hanoi

        for (i = 0; i < NUM_DISKS; i++)
            push(1, NUM_DISKS - i);  // Push disks onto initial peg
        mov(NUM_DISKS, 1, 3);  // Solve Tower of Hanoi

        glutMainLoop();
        return 0;
    }
    else {
        exit(0);
    }
}
