#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <glut.h>   // freeglut.h might be a better alternative, if available.
#include <cstdlib>
#include <map>
#include <time.h>

/*
    ISSUES:
    window resizing using winx, winy
    Generating spiral type poly's randomly
    draw edges in plotMode
    repeat option
    define macros for vec(vec(...
*/

#define PI 3.1415926535898

//Colors
GLfloat red[3] = { 1.0, 0.0, 0.0 };
GLfloat green[3] = { 0.0, 1.0, 0.0 };
GLfloat blue[3] = { 0.0, 0.0, 1.0 };
GLfloat black[3] = { 0.0, 0.0, 0.0 };
GLfloat white[3] = { 1.0, 1.0, 1.0 };
GLfloat gray[3] = { 0.5, 0.5, 0.5 };
GLfloat yellow[3] = { 1.0, 1.0, 0.0 };
GLfloat magenta[3] = { 1.0, 0.0, 1.0 };
GLfloat cyan[3] = { 0.0, 1.0, 1.0 };

using namespace std;

/*vector<vector<int>> points = {{0, 0}, {2, -2}, {4, -1}, {4, 2}, {2, -1}, {0, 2}}; */
/*vector<vector<int>> points = {{0, 0}, {50, 0}, {100, 50}, {50, 50}, {50, 100}, {20, 20}}; */
vector<vector<int>> points;
/*= { {8 ,  0},
    { 7  ,10 },
    { 6  ,0 },
    { 5  ,10 },
    { 4  ,0 },
    { 3  ,10 },
    { 2  ,  0 },
    { 1,  10 },
    {0, 0},
    { 4, -2 }};
*/

int SCALE = 1;
int n = points.size();
char ch; //choice of triang or chull
vector<int> adj, ctrlPoint; //adj[i] = next vertex of i in counterclockwise order

vector<vector<int>> chull; // Convex Hull Points

bool plotMode = false; 
bool randMode = true;

//edges are poly's original edges
//diags contains ear diags
vector<vector<vector<int>>> diags, edges;

//coordinates of last mouse click initialized to a point outside the window
float pointX = -10, pointY = -10;
float WIN_SIZE = 500;

//void triangulate();

bool angleComp(vector<int> p1, vector<int> p2) {
    int x1 = p1[0] - ctrlPoint[0];
    int y1 = p1[1] - ctrlPoint[1];
    int x2 = p2[0] - ctrlPoint[0];
    int y2 = p2[1] - ctrlPoint[1];

    if (y1 * x2 == x1 * y2) return p1[0] < p2[0];
    return (y1 * x2 < x1 * y2);
}

bool rectComp(vector<int> p1, vector<int> p2) {
    if (p1[0] == p2[0]) return p1[1] < p2[1];
    return p1[0] < p2[0];
}

void scale(int f) {
    int n = points.size();
    for (int i = 0; i < n; i++) {
        points[i][0] *= f;
        points[i][1] *= f;
    }
}

vector<vector<int>> randomPoly(vector<int> origin, int win_size, int num_points) {
    map<vector<int>, int> isExist;
    vector<vector<int>> pts;
    
    srand(time(0)); //Seed is current time
    
    int px, py;

    while(pts.size() < num_points - 1) { //Should be pts.size
        px = origin[0] + rand() % win_size;
        py = origin[1] + rand() % win_size;
        if (!isExist[{px, py}]) {
            pts.push_back({ px, py });
        }
        isExist[{px, py}]++;
    }

    sort(pts.begin(), pts.end(), angleComp);

    pts.push_back(ctrlPoint);

    return pts;
}

int Sign(vector<int> p, vector<vector<int>> l) {
    int a =  (l[1][0] - l[0][0]) * (p[1] - l[0][1]) - (l[1][1] - l[0][1]) * (p[0] - l[0][0]);

    if (a == 0) return 0;

    return a / abs(a);
}

bool isConvex(vector<int> p0, vector<int> p1, vector<int> p2) {
    vector<vector<int>> l0 = { {p0[0], p0[1]}, {p1[0], p1[1]} };
    vector<vector<int>> l1 = { {p1[0], p1[1]}, {p2[0], p2[1]} };
    //cout << Sign(p2, l0) << '\n';
    if (Sign(p2, l0) == 1) {
        return true;
    }

    else {
        return false;
    }
}

int isIntersect(vector<vector<int>> l0, vector<vector<int>> l1) {
    vector<int> p0 = l0[0], p1 = l0[1], p2 = l1[0], p3 = l1[1];
    int t0 = Sign(p0, l1), t1 = Sign(p1, l1), t2 = Sign(p2, l0), t3 = Sign(p3, l0);
    int x = t0 * t1 + t2 * t3;
    if (x == -2) {
        return 1; //Criss Cross
    }

    else if (x == -1) {
        return 0; //Intersect at point
    }

    else return 0; //No intersection
}

void Hull(vector<vector<int>> points) {
    chull.clear();

    sort(points.begin(), points.end(), rectComp);
    ctrlPoint = points[0];
    int n = points.size();
    sort(points.begin(), points.end(), angleComp);

    for (auto p : points) {
        cout << p[0] << " " << p[1] << '\n';
    }
    //chull = { points[0], points[1] };

    for (int i = 0; i < n; i++) {
        
        while (chull.size() >= 3) {
            int s = chull.size();
            vector<int> p0 = chull[s - 2], p1 = chull[s - 1], p2 = points[i];
            vector<vector<int>> l = { p0, p1 };
            if (Sign(p2, l) == -1) {
                chull.pop_back();
            }

            else break;
        }

        chull.push_back(points[i]);
    }

}


void triangulate() {
    int n = points.size();
    int cnt = 0;
    int i = 0;
    int no_diag_count = 0;

    while (cnt < n - 3 && no_diag_count < n + 1) {
        //cout << "Current Vertex: " << i << '\n';
        int prev = i % n;
        int curr = adj[prev];
        int next = adj[curr];

        bool draw = true;
        vector<vector<int>> curr_diag = { points[prev], points[next] };
        //cout << Sign(points[prev], points[curr]) << " " << Sign(points[curr], points[next]) << '\n';
        //cout << "Current Vertex: " << curr << '\n';
        if (isConvex(points[prev], points[curr], points[next])) {
            //cout << curr << '\n';
            for (auto l : edges) { //Check Intersection with Sides
                if (isIntersect(l, curr_diag)) {
                    draw = false;

                    //cout << "Intersection with side " << i << " " << isIntersect(l, { points[i], points[adj[i]] }) << '\n';
                }
            }

            if (draw) {
                //cout << "Diag added!" << '\n';
                adj[prev] = next;
                diags.push_back(curr_diag);
                cnt++;
                no_diag_count = 0;
            }

            else {
                no_diag_count++;
            }
        }

        else {
            no_diag_count++;
        }

        i = adj[i];
    }

}


void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluOrtho2D( left, right, bottom, top );
    // Centre Origin: gluOrtho2D(-1*(GLdouble)w/2, (GLdouble)w/2, -1*(GLdouble)h/2, (GLdouble)h/2);
    gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
}

void draw_circle(double r, double x, double y) {
    glColor3fv(blue);
    int precision = 100;
    double theta = 0;
    glBegin(GL_POLYGON);
    for (int i = 0; i < precision; i++) {
        theta = i * 2 * PI / precision;
        glVertex2f(x + r * cos(theta), y + r * sin(theta));
    }
    glEnd();
}

void mouse(int button, int state, int x, int y)
{
    if (plotMode && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        pointX = (float)x;  // window_width * world_width;
        pointY = (float)(WIN_SIZE - y); // window_height * world_height;

        pointX = round(pointX);
        pointY = round(pointY);
        
        cout << pointX << " " << pointY << '\n';
        // Add a vertex to the list of vertices...
        points.push_back({int(pointX), int(pointY)});
        
        glutPostRedisplay();
    }

    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        if (ch == 'y') {
            int n = points.size();
            for (int i = 0; i < n; i++) {
                adj.push_back((i + 1) % n);
            }

            for (int i = 0; i < n; i++) {
                edges.push_back({ points[i], points[adj[i]] });
            }
            /*
            if (randMode) {
                for (auto pts : points) {
                    cout<< pts[0] << ", " << pts[1] << '\n';
                }
            }
            */

            // automatically calls the display function
            clock_t start, end;
            start = clock();

            triangulate();
            end = clock();
            // Calculating total time taken by the program.
            double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
            cout << "Time taken by program is : " << fixed
                << time_taken;
            cout << " sec " << endl;
            plotMode = false;
        }

        else {
            Hull(points);

            cout << '\n';
            for (auto p : chull) {
                cout << p[0] << " " << p[1] << '\n';
            }

            cout << chull.size() << '\n';
            plotMode = false;
        }
        glutPostRedisplay();
        
        
    }
}

void display() {  // Display function will draw the image.
    
    
    glClearColor(1, 1, 1, 1);  // (In fact, this is the default.)
    glClear(GL_COLOR_BUFFER_BIT);

    if (plotMode) {
        for (auto p : points) {
            draw_circle(4, p[0], p[1]);
            /*
            glBegin(GL_LINES);
            glColor3f(1.0, 0.0, 0.0);
            glVertex2f(l[0][0], l[0][1]);
            glVertex2f(p[0], p[1]);
            glEnd();
            */
        }
    }

    else {
        if (ch == 'y') {
            glBegin(GL_LINES);
            glColor3f(1.0, 0.0, 1.0);

            for (auto l : diags) {
                glVertex2f(l[0][0], l[0][1]);
                glVertex2f(l[1][0], l[1][1]);
            }

            glEnd();


            glBegin(GL_LINE_LOOP);
            glColor3f(1.0, 0.0, 0.0);
            for (int i = 0; i < points.size(); i++) {
                glVertex2f(points[i][0], points[i][1]);
            }
            glEnd();

            for (auto p : points) {
                draw_circle(2, p[0], p[1]);
            }
        }

        else {
            glBegin(GL_LINE_LOOP);
            glColor3f(1.0, 0.0, 0.0);
            for (int i = 0; i < chull.size(); i++) {
                glVertex2f(chull[i][0], chull[i][1]);
            }
            glEnd();

            for (auto p : points) {
                draw_circle(2, p[0], p[1]);
            }
        }
    }

    
    //glutSwapBuffers(); // Required to copy color buffer onto the screen.
    glFlush();
}

int main(int argc, char** argv) {  // Initialize GLUT and 

    bool exit = false;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);    // Use single color buffer and no depth buffer.
    glutInitWindowSize(WIN_SIZE, WIN_SIZE);        // Size of display area, in pixels.
    glutInitWindowPosition(500, 100);    // Location of window in screen coordinates.
    glutCreateWindow("Triangulization"); // Parameter is window title.
    
    glutMouseFunc(mouse);
    glutDisplayFunc(display);            // Called when the window needs to be redrawn.
    glutReshapeFunc(reshape);            // Resizing coord sys with window

    char res;

    cout << "Triangulation or Convex Hull (y / n)?" << '\n';
    cin >> ch;

    cout << "Plot or Random (y / n)? " << '\n';
    cin >> res;
    if (res == 'y') {
        plotMode = true;
        randMode = false;
    }

    int num_points = 40;
    int box_size = 400;

    srand(time(0));

    ctrlPoint = { rand() % 50, rand() % 50 };
    if (randMode) {
        cout << "Enter no of vertices: " << '\n';
        cin >> num_points;
        points = randomPoly(ctrlPoint, box_size, num_points);
    }
    
    scale(SCALE);

    //triangulate();

    //cout << diags.size() << '\n';

    // Program ends when user closes the window.
    glutMainLoop(); // Run the event loop!  This function does not return.
    
    return 0;

}