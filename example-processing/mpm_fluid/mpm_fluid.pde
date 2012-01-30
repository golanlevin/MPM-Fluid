/*
 * MPM FLuid Simulation
 * Processing 2.0a4 version by Golan Levin
 * http://www.flong.com
 
 * Original Java version:
 * http://grantkot.com/MPM/Liquid.html
 
 * Flash version:
 * Copyright iunpin ( http://wonderfl.net/user/iunpin )
 * MIT License ( http://www.opensource.org/licenses/mit-license.php )
 * Downloaded from: http://wonderfl.net/c/6eu4
 
 * Javascript version:
 * Copyright Stephen Sinclair (radarsat1) ( http://www.music.mcgill.ca/~sinclair )
 * MIT License ( http://www.opensource.org/licenses/mit-license.php )
 * Downloaded from: http://www.music.mcgill.ca/~sinclair/blog
 */
 


float settings[] = {
  2.5F, /* "Density" */ 
  0.5F, /* "Stiffness" */
  3.0F, /* "Bulk Viscosity" */
  1.0F, /* "Elasticity" */
  1.0F, /* "Viscosity" */
  1.0F, /* "Yield Rate" */
  0.004F, /* "Gravity" */ //0.02
  1.0F  /* "Smoothing" */
};


Particle particles[];
int particlesX; 
int particlesY;
int nParticles;

Node grid[][];
Node activeNodeArray[];
int nActiveNodes;


int gsizeX;
int gsizeY;

boolean prevMousePressed = false;
float prevMouseX = 0;
float prevMouseY = 0; 
int mode = -1;
float SC = 10.0; //scaling

boolean bUseSettingsForJavascript = false; // expect reduced performance


//============================================================================
void setup() {
  size (400, 600, P3D);

  int np = (bUseSettingsForJavascript)? 25:50;
  if (bUseSettingsForJavascript){
    settings[0] = 1.2;
  }
  
  initLiquid (1+(int)(width/SC), 1+(int)(height/SC), np,np);
  initGridAndParticles ();

  // frameRate(30); // skip it, just run as fast as possible
  // smooth(); //actually, it's nicer without! twinkly...
}

//----------------------------------------------------------------------------
void draw() {
  background (0,0,0);

  simulate();
  render();

}



//----------------------------------------------------------------------------
void initLiquid (int ingsizeX, int ingsizeY, int inparticlesX, int inparticlesY) {

  gsizeX     = ingsizeX;
  gsizeY     = ingsizeY;
  particlesX = inparticlesX;
  particlesY = inparticlesY;

  prevMousePressed = false;
  prevMouseX = 0; 
  prevMouseY = 0;
}

//----------------------------------------------------------------------------
void initGridAndParticles() {

  grid = new Node[gsizeX][gsizeY];
  for (int i=0; i<gsizeX; i++) {
    for (int j=0; j<gsizeY; j++) {
      grid[i][j] = new Node();
    }
  }

  activeNodeArray = new Node[gsizeX * gsizeY];
  nActiveNodes = 0; 


  nParticles = particlesX * particlesY;
  particles = new Particle [nParticles]; 
  int np = 0; 
  float fi = (float ) gsizeX / (float)(particlesX+5);
  for (int i=0; i<particlesX; i++) {
    for (int j=0; j<particlesY; j++) {
      float px = (i+1)*fi;
      float py = map(j, 0,particlesY-1, 2,gsizeY/3);
      Particle p = new Particle ((i+1)*fi, py, 0.0, 0.0);
      particles[np] = p;
      np++;
    }
  }
}

//----------------------------------------------------------------------------
void render() {

  noSmooth();
  stroke (255,255,255); 
  beginShape(POINTS); 
  for (int ip=0; ip<nParticles; ip++) {
    Particle p = particles[ip];
    vertex (SC*p.x, SC*p.y);
  }
  endShape();


}

//----------------------------------------------------------------------------
void simulate () {
  boolean drag = false;
  float mdx = 0.0;
  float mdy = 0.0;
  float mx = mouseX/SC;
  float my = mouseY/SC;

  float phi;

  if (mousePressed && prevMousePressed) {
    drag = true;
    mdx = (mouseX - prevMouseX)/SC;
    mdy = (mouseY - prevMouseY)/SC;
  }

  prevMousePressed = mousePressed;
  prevMouseX = mouseX;
  prevMouseY = mouseY;

  for (int i=0; i<gsizeX; i++) {
    for (int j=0; j<gsizeY; j++) {
      grid[i][j].clear();
    }
  }
  nActiveNodes = 0; 

  //-------------------------
  // Particles pass 1
  int pcxTmp, pcyTmp;
  for (int ip=0; ip<nParticles; ip++) {

    Particle p = particles[ip];

    pcxTmp = (int)(p.x - 0.5F);
    pcxTmp = min(gsizeX-4, max(0, pcxTmp));
    int pcx = p.cx = pcxTmp;

    pcyTmp = (int)(p.y - 0.5F);
    pcyTmp = min(gsizeY-4, max(0, pcyTmp));
    int pcy = p.cy = pcyTmp;

    float px[] = p.px;
    float py[] = p.py;
    float gx[] = p.gx;
    float gy[] = p.gy;
    float pu = p.u;
    float pv = p.v;

    float x = (float) p.cx - p.x;
    px[0] = 0.5F * x * x + 1.5F * x + 1.125F;
    gx[0] = x + 1.5F;
    x++;
    px[1] = -x * x + 0.75F;
    gx[1] = -2F * x;
    x++;
    px[2] = (0.5F * x * x - 1.5F * x) + 1.125F;
    gx[2] = x - 1.5F;

    float y = (float) p.cy - p.y;
    py[0] = 0.5F * y * y + 1.5F * y + 1.125F;
    gy[0] = y + 1.5F;
    y++;
    py[1] = -y * y + 0.75F;
    gy[1] = -2F * y;
    y++;
    py[2] = (0.5F * y * y - 1.5F * y) + 1.125F;
    gy[2] = y - 1.5F;


    for (int i = 0; i < 3; i++) {
      Node nrow[] = grid[pcx + i]; // potential for array index out of bounds here if simulation explodes.
      float pxi = px[i];
      float gxi = gx[i];

      for (int j = 0; j < 3; j++) {
        Node n = nrow[pcy + j]; // potential for array index out of bounds here if simulation explodes.

        if (!n.active) {
          n.active = true;
          activeNodeArray[nActiveNodes] = n;
          nActiveNodes++;
        }
        phi   = pxi * py[j];
        n.m  += phi;
        n.gx += gxi * py[j];
        n.gy += pxi * gy[j];
        n.u  += phi * pu;
        n.v  += phi * pv;
      }
    }
  }


  for (int ni=0; ni<nActiveNodes; ni++) {
    Node n = activeNodeArray[ni]; 
    if (n.m > 0.0) {
      n.u /= n.m;
      n.v /= n.m;
    }
  }

  //-------------------------
  // Particles pass 2
  float densitySetting = settings[0];
  float stiffness      = settings[1];
  float bulkViscosity  = settings[2];
  float elasticity     = settings[3];
  float viscosity      = settings[4];
  float yieldRate      = settings[5];

  float T = millis()/1500.0;
  for (int ip=0; ip<nParticles; ip++) {
    Particle p = particles[ip];
    float px[] = p.px;
    float py[] = p.py;
    float gx[] = p.gx;
    float gy[] = p.gy;
    int    pcy = p.cy;
    int    pcx = p.cx;

    float dudx = 0.0F;
    float dudy = 0.0F;
    float dvdx = 0.0F;
    float dvdy = 0.0F;

    float gxi, pxi;
    float gxf, gyf;
    for (int i = 0; i < 3; i++) {
      Node nrow[] = grid[pcx + i];
      gxi = gx[i];
      pxi = px[i];

      Node n0 = nrow[pcy];
      gxf = gxi * py[0];
      gyf = pxi * gy[0];
      dudx += n0.u * gxf;
      dudy += n0.u * gyf;
      dvdx += n0.v * gxf;
      dvdy += n0.v * gyf;

      Node n1 = nrow[pcy+1];
      gxf = gxi * py[1];
      gyf = pxi * gy[1];
      dudx += n1.u * gxf;
      dudy += n1.u * gyf;
      dvdx += n1.v * gxf;
      dvdy += n1.v * gyf;

      Node n2 = nrow[pcy+2];
      gxf = gxi * py[2];
      gyf = pxi * gy[2];
      dudx += n2.u * gxf;
      dudy += n2.u * gyf;
      dvdx += n2.v * gxf;
      dvdy += n2.v * gyf;
    }

    float w1  = dudy - dvdx;
    float wT0 = w1 * p.T01;
    float wT1 = 0.5F * w1 * (p.T00 - p.T11);
    float D00 = dudx;
    float D01 = 0.5F * (dudy + dvdx);
    float D11 = dvdy;
    float trace = 0.5F * (D00 + D11);
    D00 -= trace;
    D11 -= trace;

    p.T00 += (-wT0 + D00) - yieldRate * p.T00;
    p.T01 += ( wT1 + D01) - yieldRate * p.T01;
    p.T11 += ( wT0 + D11) - yieldRate * p.T11;
    float norma = p.T00 * p.T00 + 2.0F * p.T01 * p.T01 + p.T11 * p.T11;
    if (norma > 5.0F) {
       p.T00 = p.T01 = p.T11 = 0.0F;
    }

    int cx = (int) p.x;
    int cy = (int) p.y;
    int cxi = cx + 1;
    int cyi = cy + 1;
    Node n00  = grid[cx ][cy ];
    Node n01  = grid[cx ][cyi];
    Node n10  = grid[cxi][cy ];
    Node n11  = grid[cxi][cyi];

    float p00 = n00.m;
    float x00 = n00.gx;
    float y00 = n00.gy;
    float p01 = n01.m;
    float x01 = n01.gx;
    float y01 = n01.gy;
    float p10 = n10.m;
    float x10 = n10.gx;
    float y10 = n10.gy;
    float p11 = n11.m;
    float x11 = n11.gx;
    float y11 = n11.gy;

    float pdx = p10 - p00;
    float pdy = p01 - p00;
    float C20 =  3.0F * pdx - x10 - 2.0F * x00;
    float C02 =  3.0F * pdy - y01 - 2.0F * y00;
    float C30 = -2.0F * pdx + x10 + x00;
    float C03 = -2.0F * pdy + y01 + y00;
    float csum1 = p00 + y00 + C02 + C03;
    float csum2 = p00 + x00 + C20 + C30;
    float C21 =   3.0F * p11 - 2.0F * x01 - x11 - 3.0F * csum1  - C20;
    float C31 = (-2.0F * p11 +        x01 + x11 + 2.0F * csum1) - C30;
    float C12 =   3.0F * p11 - 2.0F * y10 - y11 - 3.0F * csum2  - C02;
    float C13 = (-2.0F * p11 +        y10 + y11 + 2.0F * csum2) - C03;
    float C11 = x01 - C13 - C12 - x00;

    float u  = p.x - (float)cx;
    float u2 = u * u;
    float u3 = u * u2;
    float v  = p.y - (float)cy;
    float v2 = v * v;
    float v3 = v * v2;
    float density = p00 + x00 * u + y00 * v + 
      C20 * u2 + 
      C02 * v2 + 
      C30 * u3 + 
      C03 * v3 + 
      C21 * u2 * v  + 
      C31 * u3 * v  +
      C12 * u  * v2 + 
      C13 * u  * v3 + 
      C11 * u  * v  ;

    float DS = densitySetting;
    /*
    // spatially varying density field: 
    float DS = densitySetting * ( pow(  (p.y / (float) gsizeY), 3.0) ); //densitySetting
    */
    
    
    float pressure = (stiffness / max (1.0F, DS)) * (density - DS);

    if (pressure > 2.0F) {
      pressure = 2.0F;
    }
    float fx = 0.0F;
    float fy = 0.0F;
    if (p.x < 3.0F) {  
      fx += 3.0F - p.x;
    }
    else if (p.x > (float)(gsizeX - 4)) {
      fx += (gsizeX - 4.0) - p.x;
    }

    if (p.y < 3F) {
      fy += 3F - p.y;
    }
    else if (p.y > (float)(gsizeY - 4)) {
      fy += (gsizeY - 4.0) - p.y;
    }

    trace *= settings[1];
    float T00 = elasticity * p.T00 + viscosity * D00 + pressure + bulkViscosity * trace;
    float T01 = elasticity * p.T01 + viscosity * D01;
    float T11 = elasticity * p.T11 + viscosity * D11 + pressure + bulkViscosity * trace;

    float dx, dy;
    for (int i=0; i<3; i++) {
      Node nrow[] = grid[pcx + i];
      float ppxi = px[i];
      float pgxi = gx[i];

      Node n0 = nrow[pcy    ];
      phi = ppxi * py[0];
      dx  = pgxi * py[0];
      dy  = ppxi * gy[0];
      n0.ax += fx * phi -(dx * T00 + dy * T01);
      n0.ay += fy * phi -(dx * T01 + dy * T11);

      Node n1 = nrow[pcy + 1];
      phi = ppxi * py[1];
      dx  = pgxi * py[1];
      dy  = ppxi * gy[1];
      n1.ax += fx * phi -(dx * T00 + dy * T01);
      n1.ay += fy * phi -(dx * T01 + dy * T11);

      Node n2 = nrow[pcy + 2];
      phi = ppxi * py[2];
      dx  = pgxi * py[2];
      dy  = ppxi * gy[2];
      n2.ax += fx * phi -(dx * T00 + dy * T01);
      n2.ay += fy * phi -(dx * T01 + dy * T11);
    }
  }


  for (int ni=0; ni<nActiveNodes; ni++) {
    Node n = activeNodeArray[ni]; 
    if (n.m > 0.0F) {
      n.ax /= n.m;
      n.ay /= n.m;
      n.u = 0.0; 
      n.v = 0.0;
    }
  }

  //-------------------------
  // Particles pass 3
  float gravity = settings[6];
  for (int ip=0; ip<nParticles; ip++) {
    Particle p = particles[ip];

    float px[] = p.px;
    float py[] = p.py;
    int pcy = p.cy;
    int pcx = p.cx;
    for (int i=0; i<3; i++) {
      Node nrow[] = grid[pcx + i];
      float ppxi = px[i];

      Node n0 = nrow[pcy    ];
      phi = ppxi * py[0];
      p.u += phi * n0.ax;
      p.v += phi * n0.ay;

      Node n1 = nrow[pcy + 1];
      phi = ppxi * py[1];
      p.u += phi * n1.ax;
      p.v += phi * n1.ay;

      Node n2 = nrow[pcy + 2];
      phi = ppxi * py[2];
      p.u += phi * n2.ax;
      p.v += phi * n2.ay;
    }

    p.v += gravity;
    if (drag) {
      float vx = abs(p.x - mx);
      float vy = abs(p.y - my);
      if (vx < 10F && vy < 10F) {
        float weight = (1.0F - vx / 10F) * (1.0F - vy / 10F);
        p.u += weight * (mdx - p.u);
        p.v += weight * (mdy - p.v);
      }
    }

    float xf = p.x + p.u;
    float yf = p.y + p.v;
    if (xf < 2.0F) {
      p.u += (2.0F - xf) + random(0.01);
    }
    else if (xf > (gsizeX - 3.0)) {
      p.u += (gsizeX - 3.0) - xf - random(0.01);
    }
    if (yf < 2.0F) {
      p.v += (2.0F - yf) + random(0.01);
    }
    else if (yf > (gsizeY - 3.0)) {
      p.v += (gsizeY - 3.0) - yf - random(0.01);
    }


    float pu = p.u;
    float pv = p.v;
    for (int i=0; i<3; i++) {
      Node nrow[] = grid[pcx + i];
      float ppxi = px[i];

      Node n0 = nrow[pcy    ];
      phi = ppxi * py[0];
      n0.u += phi * pu;
      n0.v += phi * pv;

      Node n1 = nrow[pcy + 1];
      phi = ppxi * py[1];
      n1.u += phi * pu;
      n1.v += phi * pv;

      Node n2 = nrow[pcy + 2];
      phi = ppxi * py[2];
      n2.u += phi * pu;
      n2.v += phi * pv;
    }
  }

  for (int ni=0; ni<nActiveNodes; ni++) { 
    Node n = activeNodeArray[ni]; 
    if (n.m > 0.0F) {
      n.u /= n.m;
      n.v /= n.m;
    }
  }

  //-------------------------
  // Particles pass 4
  float gu, gv;
  float smoothing = settings[7];
  for (int ip=0; ip<nParticles; ip++) {
    Particle p = particles[ip];


    gu = 0.0F;
    gv = 0.0F;

    float px[] = p.px;
    float py[] = p.py;
    int pcy = p.cy;
    int pcx = p.cx;
    for (int i=0; i<3; i++) {
      Node nrow[] = grid[pcx + i];
      float ppxi = px[i]; 

      Node n0 = nrow[pcy    ];
      phi = ppxi * py[0];
      gu += phi * n0.u;
      gv += phi * n0.v;

      Node n1 = nrow[pcy + 1];
      phi = ppxi * py[1];
      gu += phi * n1.u;
      gv += phi * n1.v;

      Node n2 = nrow[pcy + 2];
      phi = ppxi * py[2];
      gu += phi * n2.u;
      gv += phi * n2.v;
    }

    p.x += (p.gu = gu);
    p.y += (p.gv = gv);
    p.u += smoothing * (gu - p.u);
    p.v += smoothing * (gv - p.v);
  }
  //
}




//============================================================================
class Node {

  float m;
  float d;
  float gx;
  float gy;
  float u;
  float v;
  float ax;
  float ay;
  boolean active;

  Node () {
    m  = 0;
    d  = 0;
    gx = 0;
    gy = 0;
    u  = 0;
    v  = 0;
    ax = 0;
    ay = 0;
    active = false;
  }

  void clear () {
    m = d = gx = gy = u = v = ax = ay = 0.0;
    active = false;
  }
}


//============================================================================
class Particle {

  float x    = 0;
  float y    = 0;
  float u    = 0;
  float v    = 0;

  int   cx   = 0;
  int   cy   = 0;

  float gu;
  float gv;
  float T00;
  float T01;
  float T11;

  float px[];
  float py[];
  float gx[];
  float gy[];

  Particle ( float inx, float iny, float inu, float inv) {
    x   = inx;
    y   = iny;
    u   = inu;
    v   = inv;

    px = new float[3];
    py = new float[3];
    gx = new float[3];
    gy = new float[3];
  }
}

