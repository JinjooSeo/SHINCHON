//integrates particle distribution over all azimuthal angles, rather than just 0 to pi/2 as in convert.cpp

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <convert.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_multifit.h>


int probon=0;

using namespace std;

const double fmtoGeV=5.0677;

double SCAL;

// for PCE
  int ntemp = 500;
  int particlemax = 319;
//   double *chem;
//   double *Tii;
  double chem[319][500];
  double Tii[500];
  fstream chemfile;
  //
//   double twomin = 0.0;
//   double twomax = -100;
//   double threemin = 0.0;
//   double threemax = -100;

int NUMT=8;
int STEPS=4000,UPDATE=100,SNAPUPDATE=1000;
double AT=0.05,EPS=0.001,B=0.0;
double ETAOS=0.3;
double TSTART=0.5,TF=0.1,TINIT=1.0;
double IC;
int PTASIZE=100;
int PHIPASIZE=4;
const double INTACC=1.e-2;
const int INTSPACE=2000;

//controls value of tau_Pi
double COEFF=3.0;


double L1COEF=2.0;
double L2COEF;
int FREEZE=1;
char EOSNAME[255];
char ETANAME[255];
char ZETANAME[255];
char BETANAME[255];
char LAMBDANAME[255];
long int B3DEVENTS;
double PTMAX, TRIEPS = 0.0, TRIANGLE = 0.0;
double QUADEPS = 0.0, QUADANGLE = 0.0;
double QUINTEPS = 0.0, QUINTANGLE = 0.0;
double SEXEPS = 0.0, SEXANGLE = 0.0;
double SEPTEPS = 0.0, SEPTANGLE = 0.0;
double BIEPS = 0.0, BIANGLE = 0.0;
double MONOEPS, MONOANGLE;
int FULL = 0;
int PCE = 0;
int NS=0, IFLOW=0;

int preeqflow;

double RNUC, ANUC, SIGMANN, TANORM;

int SMOOTHING=0;
double SMOOTH;

double *xp,*yp,*ux,*uy,*pixx,*pixy,*piyy,*phi;
double *taup, *Tp;
int *direction;
double *taus;

//for the equation of state
double *eoT4,*cs2i,*poT4,*Ti;
double *Te, *Co;
//defined in loadeos
int length;
int alphalength = 100;
//to know where to stop interpolation
double lowestE;

//   double argmin = 2.0;
//   double argmax = 0.0;

//splines
gsl_spline **xspline;
gsl_interp_accel **xacc;
gsl_spline **yspline;
gsl_interp_accel **yacc;
gsl_spline **uxspline;
gsl_interp_accel **uxacc;
gsl_spline **uyspline;
gsl_interp_accel **uyacc;
gsl_spline **pixxspline;
gsl_interp_accel **pixxacc;
gsl_spline **pixyspline;
gsl_interp_accel **pixyacc;
gsl_spline **piyyspline;
gsl_interp_accel **piyyacc;
gsl_interp_accel *tsacc; 
gsl_spline *tsspline;
gsl_interp_accel **dtxacc; 
gsl_spline **dtxspline;
gsl_interp_accel **dtyacc; 
gsl_spline **dtyspline;


//splines for last 10%
gsl_spline **tspline;
gsl_interp_accel **tacc;
gsl_spline **ouxspline;
gsl_interp_accel **ouxacc;
gsl_spline **ouyspline;
gsl_interp_accel **ouyacc;
gsl_spline **opixxspline;
gsl_interp_accel **opixxacc;
gsl_spline **opixyspline;
gsl_interp_accel **opixyacc;
gsl_spline **opiyyspline;
gsl_interp_accel **opiyyacc;
gsl_spline **dxtspline;
gsl_interp_accel **dxtacc;


//splines -- for equation of state
gsl_interp_accel *pacc,*eacc;
gsl_spline *pspline,*espline;

//spline for delta f coefficient as a function of temperature
gsl_interp_accel *alphaacc;
gsl_spline *alphaspline;

//spline for chemical potentials when using PCE EOS
gsl_interp_accel *chemacc;
gsl_spline *chemspline;

struct dummy_params
{
  gsl_spline * myspline;
  gsl_interp_accel * myacc;
};




gsl_integration_workspace * w = gsl_integration_workspace_alloc (INTSPACE);

//const int LIMITS=1024;
const int LIMITS=5200;
const int MAXL=1024;
const int POLYORD=10;
const int maxline = 128; // maximum line length used in the buffer for reading

//const int PTASIZE=100;  // how many grid points in PT
//const int PHIPASIZE=12;   // how many grid points in PHIP

int numset[LIMITS];
int totalnum,switcher;
int subnum;
int middle,numpoints;

double stepper;

int * counterarr;
double *boundarr;

int switcherpercent=80;
// double PTMAX=4.2;


// input files
fstream freeze_out,dummy;

fstream massfile,namesfile,gsfile;

// output files
fstream pttab,ptfile,testfile;


double getchem(int i, double t)
{
  return 0.0;
}

//Find out how many sets
void countsets()
{
  char buffer[1024];
  int limiter=0;
  totalnum=0;
  subnum=-1;
  //determine how many sets
  while (!freeze_out.eof())
    {
      freeze_out.getline(buffer,1024,'\n');
      subnum++;
      if (buffer[0]=='T')
	{
	  subnum--;
	  //printf("line= %s\n",buffer);
	  //printf("found at %i\n",limiter);
	  numset[totalnum]=limiter;
	  //printf("got %i\n",numset[totalnum]);
	  totalnum++;
	  if (totalnum>LIMITS)
	    {
	      printf("More than LIMITS sets. Aborting\n");
	      break;
	    }
	}
      limiter++;
    }
  printf("Found %i data lines \n",subnum);

  //totalnum--;

  //determine switching point
  //last ten percent of data points roughly

  switcher=switcherpercent*totalnum/100;

//   cout << "totalnum = " << totalnum << "\nswitcher = " << switcher << "\nswitcherpercent = " << switcherpercent << endl;

  printf("Switching integration at %i\n",switcher);

  numpoints=2*(totalnum-switcher);

  //make it odd
  if (numpoints%2==0)
    numpoints++;

  middle=(numpoints-1)/2;

  if (middle<5)
    {
      numpoints*=4;
      numpoints++;
      middle=(numpoints-1)/2;
    }


}

//allocate Memory
void allocMem()
{
  xp=new double[subnum];
  yp=new double[subnum];
  phi=new double[subnum];
  ux=new double[subnum];
  uy=new double[subnum];
  pixx=new double[subnum];
  pixy=new double[subnum];
  piyy=new double[subnum];
  taus=new double[totalnum];

  xspline=new gsl_spline*[totalnum];
  xacc=new gsl_interp_accel*[totalnum];
  yspline=new gsl_spline*[totalnum];
  yacc=new gsl_interp_accel*[totalnum];
  uxspline=new gsl_spline*[totalnum];
  uxacc=new gsl_interp_accel*[totalnum];
  uyspline=new gsl_spline*[totalnum];
  uyacc=new gsl_interp_accel*[totalnum];
  pixxspline=new gsl_spline*[totalnum];
  pixxacc=new gsl_interp_accel*[totalnum];
  pixyspline=new gsl_spline*[totalnum];
  pixyacc=new gsl_interp_accel*[totalnum];
  piyyspline=new gsl_spline*[totalnum];
  piyyacc=new gsl_interp_accel*[totalnum];

  dtxspline=new gsl_spline*[totalnum];
  dtxacc=new gsl_interp_accel*[totalnum];
  dtyspline=new gsl_spline*[totalnum];
  dtyacc=new gsl_interp_accel*[totalnum];
  //dtxacc=gsl_interp_accel_alloc ();
  //dtyspline=gsl_spline_alloc (gsl_interp_cspline, 3);
  // dtyacc=gsl_interp_accel_alloc ();


  tspline=new gsl_spline*[numpoints];
  tacc=new gsl_interp_accel*[numpoints];
  ouxspline=new gsl_spline*[numpoints];
  ouxacc=new gsl_interp_accel*[numpoints];
  ouyspline=new gsl_spline*[numpoints];
  ouyacc=new gsl_interp_accel*[numpoints];
  opixxspline=new gsl_spline*[numpoints];
  opixxacc=new gsl_interp_accel*[numpoints];
  opixyspline=new gsl_spline*[numpoints];
  opixyacc=new gsl_interp_accel*[numpoints];
  opiyyspline=new gsl_spline*[numpoints];
  opiyyacc=new gsl_interp_accel*[numpoints];
  dxtspline=new gsl_spline*[numpoints];
  dxtacc=new gsl_interp_accel*[numpoints];


  counterarr=new int[numpoints];
  boundarr=new double[numpoints];



  tsspline=gsl_spline_alloc (gsl_interp_cspline, switcher-1);
  tsacc=gsl_interp_accel_alloc ();
  //dtxspline=gsl_spline_alloc (gsl_interp_cspline, 3);
  //dtxacc=gsl_interp_accel_alloc ();
  //dtyspline=gsl_spline_alloc (gsl_interp_cspline, 3);
  // dtyacc=gsl_interp_accel_alloc ();
}

void blockallocMem()
{
  xp=new double[subnum];
  yp=new double[subnum];
  phi=new double[subnum];
  ux=new double[subnum];
  uy=new double[subnum];
  pixx=new double[subnum];
  pixy=new double[subnum];
  piyy=new double[subnum];
  taup=new double[subnum];
  direction = new int[subnum];
  Tp=new double[subnum];
  taus=new double[totalnum];
}

void freeMem()
{
  
  for (int i=0;i<totalnum;i++)
    {
      gsl_spline_free (xspline[i]);
      gsl_interp_accel_free (xacc[i]);
      gsl_spline_free (yspline[i]);
      gsl_interp_accel_free (yacc[i]);
      gsl_spline_free (uxspline[i]);
      gsl_interp_accel_free (uxacc[i]);
      gsl_spline_free (uyspline[i]);
      gsl_interp_accel_free (uyacc[i]);
      gsl_spline_free (pixxspline[i]);
      gsl_interp_accel_free (pixxacc[i]);
      gsl_spline_free (pixyspline[i]);
      gsl_interp_accel_free (pixyacc[i]);
      gsl_spline_free (piyyspline[i]);
      gsl_interp_accel_free (piyyacc[i]);
      gsl_spline_free (dtxspline[i]);
      gsl_interp_accel_free (dtxacc[i]);
      gsl_spline_free (dtyspline[i]);
      gsl_interp_accel_free (dtyacc[i]);
    }

  for (int i=0;i<numpoints;i++)
    {
      if (counterarr[i]>2)
	{
	  gsl_spline_free (tspline[i]);
	  gsl_interp_accel_free (tacc[i]);
	  gsl_spline_free (ouxspline[i]);
	  gsl_interp_accel_free (ouxacc[i]);
	  gsl_spline_free (ouyspline[i]);
	  gsl_interp_accel_free (ouyacc[i]);
	  gsl_spline_free (opixxspline[i]);
	  gsl_interp_accel_free (opixxacc[i]);
	  gsl_spline_free (opixyspline[i]);
	  gsl_interp_accel_free (opixyacc[i]);
	  gsl_spline_free (opiyyspline[i]);
	  gsl_interp_accel_free (opiyyacc[i]);
	  gsl_spline_free (dxtspline[i]);
	  gsl_interp_accel_free (dxtacc[i]);
	}
    }


  delete [] counterarr;
  delete [] boundarr;

  gsl_spline_free(tsspline);
  gsl_interp_accel_free(tsacc);

  gsl_integration_workspace_free (w);
}

//Get data
int readsets()
{
  double temp;

  //freeze_out.seekg(0);
  int dumsize=0;

  int pos=0;
  int setpos=0;
  char d[128];

  char obda;

  while((!dummy.eof())&&(setpos<totalnum))
    {
      if (pos!=(numset[setpos]-setpos))
	{
	  /*
	  dummy >> temp;
	  
	  if (pos%7==0)
	    {
	      cout << endl;
	      //length=dummy.tellg();
	      //printf("pos = %i\n",length);
	      
	    }
	  printf("%f \t",temp);
	  pos++;
	  */
	  dummy >> xp[pos];
	  dummy >> yp[pos];
	  phi[pos]=atan2(yp[pos],xp[pos]);
	  //take advantage of full 2Pi information:
	  if (phi[pos]<0) phi[pos]+=2*M_PI;
	  dummy >> ux[pos];
	  dummy >> uy[pos];
	  dummy >> pixx[pos];
	  dummy >> pixy[pos];
	  dummy >> piyy[pos];
	  // if ((yp[pos]<0)&&(xp[pos]==0)&&(setpos>switcher))
	  //  printf("{%i,%f},\n",setpos,yp[pos]);
	  //printf("%f %f %f %f %f %f %f \t %f\n",xp[pos],yp[pos],ux[pos],uy[pos],pixx[pos],pixy[pos],piyy[pos],phi[pos]);
	  pos++;
	}
      else
	{
	  dummy >> obda;
	  dummy >> obda;
	  dummy >> obda;
	  dummy >> obda;
	  dummy >> temp;
	  taus[setpos]=temp;
	  //printf("END for t= %f \n",taus[setpos]);
	  setpos++;
	  //pos+=7;
	}
    }
  cout << endl;

  return 0;
}

int blockreadsets()
{
  int pos=0;
  while((!dummy.eof()))
  {
    dummy >> xp[pos];
    dummy >> yp[pos];
    dummy >> taup[pos];
    dummy >> direction[pos];
    dummy >> ux[pos];
    dummy >> uy[pos];
    dummy >> pixx[pos];
    dummy >> pixy[pos];
    dummy >> piyy[pos];
    dummy >> Tp[pos]; 
//     if (pos==subnum-1) {cout << "pos = " << pos << 
// 			"\nxp = " << xp[pos] <<
// 			"\nyp = " << yp[pos] <<
// 			"\ntaup = " << taup[pos] <<
// 			"\ndirection = " << direction[pos] << endl;}
    pos++;

  }
  return 0;
}

double coshintegrand0(double T2, double T3, double x)
{
  double csh = cosh(x);
  return exp(T2*csh + T3*csh*csh);
}

double coshintegrand1(double T2, double T3, double x)
{
  double csh = cosh(x);
  return csh * exp(T2*csh + T3*csh*csh);
}

double xinv(double T2, double T3, double y)
{
      double rt = sqrt(T2*T2 - 4.*T3*y);
      double chx = 2.*y/(-T2 + rt);
// this is the inverse of cosh(x)     
      return log(chx+sqrt(chx*chx-1.0));
      
}

//this integrates the off-equilibrium distribution function over the (block-wise) freeze out surface
double blockintegrate(int part, double m0,double pt, double phip)
{
  double sum = 0.0;
  double px=pt*cos(phip);
  double py=pt*sin(phip);
  double mt=sqrt(pt*pt+m0*m0);

  for (int pos=0;pos<subnum;pos++)
  {
  
    double mux=ux[pos];
    double muy=uy[pos];
  
    double T = Tp[pos];
    double ut=sqrt(1+mux*mux+muy*muy);
    double d0k0=gsl_sf_bessel_K0(mt/T*ut);
    double d1k0=gsl_sf_bessel_K1(mt/T*ut);

    double temp;

    double mpixx, mpixy, mpiyy, e, p, c;

    if (FREEZE < 3 && !PCE)
    {
      mpixx=pixx[pos]/(T*T*2);
      mpixy=pixy[pos]/(T*T*2);
      mpiyy=piyy[pos]/(T*T*2);
    }
    else
    {
      //if not using quadratic ansatz for viscous correction, do the rapidity integral numerically
      e = gsl_spline_eval(espline,T,eacc);
      p = gsl_spline_eval(pspline,T,pacc);
      c = gsl_spline_eval(alphaspline,T,alphaacc);
      
      //multiply by coefficient and remove factor of enthalpy
      mpixx=c*(e+p)*pixx[pos];
      mpixy=c*(e+p)*pixy[pos];
      mpiyy=c*(e+p)*piyy[pos];
    }
      
      
    double vx=mux/ut;
    double vy=muy/ut;
    double mpitt=vx*vx*mpixx+2*vx*vy*mpixy+vy*vy*mpiyy;
    double mpiee=mpixx+mpiyy-mpitt;
    double mpitx=vx*mpixx+vy*mpixy;
    double mpity=vx*mpixy+vy*mpiyy;
    
    double one=mt*mt*mpiee+px*px*mpixx+py*py*mpiyy+2*px*py*mpixy;
    double two=-2*mt*(px*mpitx+py*mpity);
    double three=mt*mt*(mpitt-mpiee);
           
//    double d2k0=(gsl_sf_bessel_Kn(2,mt/T*ut)+d0k0)/2;
//    double d3k0=(gsl_sf_bessel_Kn(3,mt/T*ut)+3*d1k0)/4;

    double xfactor, yfactor, taufactor;
    if (FREEZE == 2 && !PCE)//quadratic ansatz
    {
      double d2k0=(gsl_sf_bessel_Kn(2,mt/T*ut)+d0k0)/2;
      double d3k0=(gsl_sf_bessel_Kn(3,mt/T*ut)+3*d1k0)/4;
      xfactor = (1+one)*d0k0+two*d1k0+three*d2k0;
      yfactor = (1+one)*d0k0+two*d1k0+three*d2k0;
      taufactor = (1+one)*d1k0+two*d2k0+three*d3k0;
    }
    else
    {
      //do rapidity integrals numerically:
      //algorithm and Fortran comments by J-Y Ollitrault
      //////////////////////////////

      // c     this program computes the integral of the exponential of cosh
      // c     over the real axis using the trapezoidal method 
      // c     with a step dx and a cutoff xmax

      // c     np is the number of integration points within the half width delta
      // c     the number of correct digits in the integral increases linearly with np
      // c     (exponential convergence)
      int np = 3;
      // c     the error is assumed to scale like exp(-alpha*np)
      // c     I have tuned alpha by hand to optimize 
      double alpha = 4.0;
    
      double delta = xinv(-mt*ut/T, 0, 1.0 + mt*ut/T);
    
      double dx = delta/np;
      double xmax=xinv(-mt*ut/T, 0, alpha*np + mt*ut/T);
      double nmax=int(xmax/dx)+1;
      
      double fint0 = 0.0;
      double fint1 = 0.0;
      double fint2 = 0.0;
      double fint3 = 0.0;
      
      switch (FREEZE)
      {
	case 2://quadratic ansatz, with PCE
	  for (int n=1;n<=nmax;n++)
	  {
	    double x = (n - 0.5)*dx;
	    double csh = cosh(x);
	    double pdotu = mt*ut*csh - px*mux - py*muy;
//	    double srt = sqrt(-m0*m0 + pdotu*pdotu);
//	    double rtsrt =  sqrt(srt);
	    fint0 += dx * exp(-mt*ut/T*csh);
	    fint1 += dx * csh * exp(-mt*ut/T*csh);
	    fint2 += dx * csh*csh * exp(-mt*ut/T*csh);
	    fint3 += dx * csh*csh*csh * exp(-mt*ut/T*csh);
	  }
	  break;
	case 3://linear ansatz
	  for (int n=1;n<=nmax;n++)
	  {
	    double x = (n - 0.5)*dx;
	    double csh = cosh(x);
	    double pdotu = mt*ut*csh - px*mux - py*muy;
	    double srt = sqrt(-m0*m0 + pdotu*pdotu);
	    fint0 += dx * exp(-mt*ut/T*csh) / srt;
	    fint1 += dx * csh * exp(-mt*ut/T*csh) / srt;
	    fint2 += dx * csh*csh * exp(-mt*ut/T*csh) / srt;
	    fint3 += dx * csh*csh*csh * exp(-mt*ut/T*csh) / srt;
	  }
	  break;
	case 4://alpha = 0.5 -- delta f ~ p^1.5
	  for (int n=1;n<=nmax;n++)
	  {
	    double x = (n - 0.5)*dx;
	    double csh = cosh(x);
	    double pdotu = mt*ut*csh - px*mux - py*muy;
	    double srt = sqrt(-m0*m0 + pdotu*pdotu);
	    double rtsrt =  sqrt(srt);
	    fint0 += dx * exp(-mt*ut/T*csh) / rtsrt;
	    fint1 += dx * csh * exp(-mt*ut/T*csh) / rtsrt;
	    fint2 += dx * csh*csh * exp(-mt*ut/T*csh) / rtsrt;
	    fint3 += dx * csh*csh*csh * exp(-mt*ut/T*csh) / rtsrt;
	  }
	  break;
	case 5://delta f ~ p log(1+p/m)
	  for (int n=1;n<=nmax;n++)
	  {
	    double x = (n - 0.5)*dx;
	    double csh = cosh(x);
	    double pdotu = mt*ut*csh - px*mux - py*muy;
	    double srt = sqrt(-m0*m0 + pdotu*pdotu);
	    double lg = log(1 + srt/m0);
	    fint0 += dx * exp(-mt*ut/T*csh) / srt * lg;
	    fint1 += dx * csh * exp(-mt*ut/T*csh) / srt * lg;
	    fint2 += dx * csh*csh * exp(-mt*ut/T*csh) / srt * lg;
	    fint3 += dx * csh*csh*csh * exp(-mt*ut/T*csh) / srt * lg;
	  }
	case 6://delta f ~ p log(1+p/T)
	  for (int n=1;n<=nmax;n++)
	  {
	    double x = (n - 0.5)*dx;
	    double csh = cosh(x);
	    double pdotu = mt*ut*csh - px*mux - py*muy;
	    double srt = sqrt(-m0*m0 + pdotu*pdotu);
	    double lg = log(1 + srt/T);
	    fint0 += dx * exp(-mt*ut/T*csh) / srt * lg;
	    fint1 += dx * csh * exp(-mt*ut/T*csh) / srt * lg;
	    fint2 += dx * csh*csh * exp(-mt*ut/T*csh) / srt * lg;
	    fint3 += dx * csh*csh*csh * exp(-mt*ut/T*csh) / srt * lg;
	  }
	  break;
	default:
	  cout << "Unknown FREEZE value " << FREEZE << endl;
	  exit(0);

      }
      
      xfactor = one*fint0+two*fint1+three*fint2 + d0k0;
      yfactor = one*fint0+two*fint1+three*fint2 + d0k0;
      taufactor = one*fint1+two*fint2+three*fint3 + d1k0;
      
    }
    
    //three possible directions for the surface segment: +-x, +-y, +-tau
    int sign;
    switch (abs(direction[pos]))
    {
      case 1:
	sign = direction[pos];
	temp = sign*px*taup[pos]*AT*AT*EPS*UPDATE;
	temp*= xfactor;
	break;
      case 2:
	sign = direction[pos]/2;
	temp = sign*py*taup[pos]*AT*AT*EPS*UPDATE;
	temp*= yfactor;
	break;
      case 3:
	sign = direction[pos]/3;
	temp = sign*mt*taup[pos]*AT*AT;
	temp*= taufactor;
	break;
      default:
	cout << "What direction for line " << pos << "?\n";
	cout << "I have " << direction[pos] << "\n";
	cout << "abs(direction) = " << abs(direction[pos]) << endl;
	exit(0);
    }

      temp*=exp((px*mux+py*muy)/T);
      if (PCE)
      {
	double cp = gsl_spline_eval(chemspline,T,chemacc);
// 	double cp = 0;
	double fugacity = exp(cp/T);
// 	cout << cp << endl;
	temp*=fugacity;
      }
      sum+=temp;
  }
  sum/=2*M_PI;
  sum/=2*M_PI;
  sum/=2*M_PI;
  sum*=2;
  sum*=fmtoGeV;

  return sum;
}

//load equation of state, and open coefficient file
void loadeos()
{
  fstream eosf;

  eosf.open("input/qcdIEOS.dat",ios::in);

  if (eosf.is_open())
    {
      eosf >> length;
      cout << "Read length of " << length << endl;

      

      eoT4 = new double[length];
      cs2i = new double[length];
      poT4 = new double[length];
      Ti = new double[length];

      for (int i=1;i<=length;i++)
	{
	  eosf >> Ti[i-1];
	  eosf >> eoT4[i-1];
	  eosf >> poT4[i-1];
	  eosf >> cs2i[i-1];
	}

      eosf.close();
    }
  else
    cout << "Could not open EOS file" << endl; 


  //get coefficient for viscous correction to distribution function (delta f)
  //previously calculated as a function of temperature in Mathematica
  switch (FREEZE)
  {
//     case 9:
//       cout << "Implementing quadratic ansatz for viscous correction\n"
//       eosf.open("alpha0coef.dat",ios::in);//quadratic ansatz coefficients
//       break;
    case 2:
      cout << "Implementing quadratic ansatz for viscous correction\n";
      if (PCE) eosf.open("input/pasialpha0coef.dat",ios::in);
      break;
    case 3:
      cout << "Implementing linear ansatz for viscous correction\n";
      if (PCE) eosf.open("input/pasialpha1coef.dat",ios::in);
      else eosf.open("input/alpha1coef.dat",ios::in);//linear ansatz coefficients
      break;
    case 4:
      cout << "Implementing viscous correction proportional to p^1.5 (alpha = 0.5)\n";
      eosf.open("input/alpha0.5coef.dat",ios::in);//in between quadratic and linear
      break;
    case 5:
      cout << "Implementing viscous correction proportional to p log(1+p/m)\n";
      eosf.open("input/alphalogcoef.dat",ios::in);
      break;
    case 6:
      cout << "Implementing viscous correction proportional to p log(1+p/T)\n";
      eosf.open("input/alphalogTcoef.dat",ios::in);
      break;
    default:
      cout << "Which FREEZE?" << FREEZE << endl;
      exit(0);
  }

  if (eosf.is_open())
    {  

      Te = new double[alphalength];
      Co = new double[alphalength];
      
      for (int i=1;i<=alphalength;i++)
	{
	  eosf >> Te[i-1];
	  eosf >> Co[i-1];
	}

      eosf.close();
    }
  else if (FREEZE > 2)
    cout << "Could not open alphacoef file" << endl;


  //interpolate
  pacc=gsl_interp_accel_alloc (); 
  eacc=gsl_interp_accel_alloc (); 
  alphaacc=gsl_interp_accel_alloc ();
  pspline=gsl_spline_alloc (gsl_interp_cspline, length);
  espline=gsl_spline_alloc (gsl_interp_cspline, length);
  alphaspline=gsl_spline_alloc (gsl_interp_cspline, alphalength);

  
  double warrx[length];
  double warry[length];

  for (int i=0;i<length;i++)
    {
      warrx[i]=Ti[i];
      warry[i]=poT4[i]*Ti[i]*Ti[i]*Ti[i]*Ti[i];
    }

  lowestE=warrx[0];

  gsl_spline_init (pspline,warrx,warry,length);

  if (FREEZE > 2 | PCE)
  {

    for (int i=0;i<length;i++)
      {
	warry[i]=eoT4[i]*Ti[i]*Ti[i]*Ti[i]*Ti[i];
      }  
    gsl_spline_init (espline,warrx,warry,length);

    gsl_spline_init (alphaspline,Te,Co,alphalength);
  }
//read in chemical potentials for each resonance
  chemacc=gsl_interp_accel_alloc ();
  chemspline=gsl_spline_alloc (gsl_interp_cspline, ntemp);
  if (PCE)
  {
    chemfile.open("input/allchem.dat", ios::in);
    for (int j=0; j<ntemp; j++)
    {
      Tii[j] = Ti[j];
      for (int i=0; i<particlemax; i++)
      {
	chemfile >> chem[i][j];
// 	if (j==0) cout << chem[i][j] << "  ";
      } 
    }
      chemfile.close();
  }

}

//Smoothing data -- important for corse lattices
void smoothdata(double *datax,double *data1y,double *data2y, int L)
{
  gsl_spline * workspline1=gsl_spline_alloc (gsl_interp_cspline_periodic, L);
  gsl_interp_accel * workacc1=gsl_interp_accel_alloc ();
  gsl_spline * workspline2=gsl_spline_alloc (gsl_interp_cspline_periodic, L);
  gsl_interp_accel * workacc2=gsl_interp_accel_alloc ();

  gsl_spline_init (workspline1, datax, data1y, L);
  gsl_spline_init (workspline2, datax, data2y, L);
  
  //for (int i=0;i<L;i++)
  //  {
  //    printf("{%f,%f},\n",datax[i],datay[i]);
  //  }

  for (int i=0;i<(L-1);i++)
    {
      double temp=i*2*M_PI/(L-1);
      data1y[i]=gsl_spline_eval(workspline1,temp,workacc1);
      data2y[i]=gsl_spline_eval(workspline2,temp,workacc2);
      //datax[i]=2*M_PI/L*i;
      //datay[i]=sin(datax[i]);
    }

  gsl_fft_real_wavetable * real;
  gsl_fft_halfcomplex_wavetable * hc;
  gsl_fft_real_workspace * work;


  work = gsl_fft_real_workspace_alloc (L-1);
  real = gsl_fft_real_wavetable_alloc (L-1);
  hc = gsl_fft_halfcomplex_wavetable_alloc (L-1);    
  //first set
  
  gsl_fft_real_transform (data1y, 1, L-1, real, work);

  //smoothing:
  //remove constant (should be centered)
  data1y[0]=0;
  
  //remove higher orders
  //if (L>15)
  //  for (int i=cut;i<L-1;i++)
  //   {
  //	data1y[i]=0;
  ////printf("%i %f",i,a[i]);
  //  }

  //remove sin and even cos parts
  for (int i=1;i<L-1;i++)
  {
    //printf("smoother ux %i %f\n",i,data1y[i]);
    if (probon==1)
      printf("%i data1y=%f",i, data1y[i]);
    if (((i-1)%4)!=0)
      data1y[i]=0;
    if (probon==1)
      printf("vs %f\n",data1y[i]);
  }

  gsl_fft_halfcomplex_inverse (data1y, 1, L-1, hc, work);
  
  //second set
  
  gsl_fft_real_transform (data2y, 1, L-1, real, work);

  data2y[0]=0;
  for (int i=1;i<L-1;i++)
  {
    //printf("smoother uy %i %f\n",i,data2y[i]);
    if (probon==1)
      printf("%i data2y=%f",i, data2y[i]);
    if (((i-2)%4)!=0)
     data2y[i]=0;
    if (probon==1)
      printf("vs %f\n",data2y[i]);
  }

  //if (L>15)
  //    for (int i=cut;i<L-1;i++)
  //  data2y[i]=0;
 
  gsl_fft_halfcomplex_inverse (data2y, 1, L-1, hc, work);


  //clean up
  gsl_fft_halfcomplex_wavetable_free (hc);
  gsl_fft_real_wavetable_free (real);
  gsl_fft_real_workspace_free (work);

  gsl_spline_free (workspline1);
  gsl_interp_accel_free (workacc1);
  gsl_spline_free (workspline2);
  gsl_interp_accel_free (workacc2);

}

//Interpolation routines

void interpolate1()
{
  const gsl_interp_type *t = gsl_interp_cspline_periodic; 

  for (int i=0;i<totalnum;i++)
    {
      int length=numset[0]+1;
      int start=0;
      if (i!=0)
	{
	  length=numset[i]-numset[i-1];
	  start=numset[i-1]-i+1;
	}
      //cout << "length=" << length;
      xacc[i] = gsl_interp_accel_alloc ();
      yacc[i] = gsl_interp_accel_alloc ();
      uxacc[i] = gsl_interp_accel_alloc ();
      uyacc[i] = gsl_interp_accel_alloc ();
      pixxacc[i] = gsl_interp_accel_alloc ();
      pixyacc[i] = gsl_interp_accel_alloc ();
      piyyacc[i] = gsl_interp_accel_alloc ();
      xspline[i] = gsl_spline_alloc (t, length);
      yspline[i] = gsl_spline_alloc (t, length);
      uxspline[i] = gsl_spline_alloc (t, length);
      uyspline[i] = gsl_spline_alloc (t, length);
      pixxspline[i] = gsl_spline_alloc (t, length);
      pixyspline[i] = gsl_spline_alloc (t, length);
      piyyspline[i] = gsl_spline_alloc (t, length);
      
      double *tempx,*tempy,*tempux,*tempuy,*temppixx,*temppixy,*temppiyy;
      tempx=new double[length];
      tempy=new double[length];
      tempux=new double[length];
      tempuy=new double[length];
      temppixx=new double[length];
      temppixy=new double[length];
      temppiyy=new double[length];
      double *tempp;
      tempp=new double[length];
      

      for (int j=0;j<length-1;j++)
	{

	  tempp[j]=phi[start+j];
	  if (j>0)
	    {
	      if (tempp[j]>tempp[j-1])
		{
		  tempx[j]=xp[start+j];
		  tempy[j]=yp[start+j];
		  tempux[j]=ux[start+j];
		  tempuy[j]=uy[start+j];
		  temppixx[j]=pixx[start+j];
		  temppixy[j]=pixy[start+j];
		  temppiyy[j]=piyy[start+j];
		}
	      else
		{
		  /*
		  tempp[j]=tempp[j-1];
		  tempp[j-1]=phi[start+j];
		  
		  tempx[j]=tempx[j-1];
		  tempx[j-1]=xp[start+j];
		  tempy[j]=tempy[j-1];
		  tempy[j-1]=yp[start+j];

		  tempux[j]=tempux[j-1];
		  tempux[j-1]=ux[start+j];
		  tempuy[j]=tempuy[j-1];
		  tempuy[j-1]=uy[start+j];

		  temppixx[j]=temppixx[j-1];
		  temppixx[j-1]=pixx[start+j];
		  temppixy[j]=temppixy[j-1];
		  temppixy[j-1]=pixy[start+j];
		  temppiyy[j]=temppiyy[j-1];
		  temppiyy[j-1]=piyy[start+j];
		  */

		  printf("WARNING: non-monotonic behaviour at %i of %i\n",i,totalnum);
		}
	    }
	  else
	    {
	      tempx[j]=xp[start+j];
	      tempy[j]=yp[start+j];
	      tempux[j]=ux[start+j];
	      tempuy[j]=uy[start+j];
	      temppixx[j]=pixx[start+j];
	      temppixy[j]=pixy[start+j];
	      temppiyy[j]=piyy[start+j];
	    }

	  
	  // if (i>switcher)
	  // printf("%i %i %f vs %f\n",i,j,ux[start+j]/uy[start+j],xp[start+j]/yp[start+j]);
	  //if ((tempx[j]==0)&&(tempy[j]<0)&&(i>switcher))
	  // printf("{%f,%f},\n",taus[i],tempy[j]);
	  
	  
	  
	  
	}
       

      /*
      for (int j=0;j<length-1;j++)
	{
	  if (i==totalnum-2)
	printf("tot2 %f %f at %f\n",tempx[j],tempy[j],tempp[j]);
	  if (i==totalnum-1)
	    printf("tot1 %i %f %f at %f\n",i,tempx[j],tempy[j],tempp[j]);
	}
      */
      
      tempx[length-1]=xp[start];
      tempy[length-1]=yp[start]; 
      tempux[length-1]=ux[start];
      tempuy[length-1]=uy[start];
      temppixx[length-1]=pixx[start];
      temppixy[length-1]=pixy[start];
      temppiyy[length-1]=piyy[start];
      tempp[length-1]=2*M_PI;
 
      
      //smoothdata(tempp,tempx,tempy,length);
      //if (i>switcher)
      //	probon=1;
      //smoothdata(tempp,tempux,tempuy,length);
      //probon=0;
      //smoothdata(tempp,tempx,tempy,length,(int) 3);

      /*
      double ddd[length];

      for (int ll=0;ll<(length-1);ll++)
	{
	  double temp=ll*2*M_PI/(length-1);
	  ddd[ll]=temp;
	}
      


      //phi-splines
      gsl_spline_init (uxspline[i], ddd, tempux, length);
      gsl_spline_init (uyspline[i], ddd, tempuy, length);
      */

      gsl_spline_init (uxspline[i], tempp, tempux, length);
      gsl_spline_init (uyspline[i], tempp, tempuy, length);
      gsl_spline_init (pixxspline[i], tempp, temppixx, length);
      gsl_spline_init (pixyspline[i], tempp, temppixy, length);
      gsl_spline_init (piyyspline[i], tempp, temppiyy, length);

      
 
 
      tempx[length-1]=tempx[0];
      tempy[length-1]=tempy[0]; 
      tempp[length-1]=2*M_PI;

      //if (i==0)
      //	{
      //	  for (int k=0;k<length;k++)
      //	    {
      //	      printf("{%f,%f},\n",tempp[k],tempx[k]);
      //	    }
      //	}
      //smoothdata(tempx,length-1);
      //smoothdata(tempy,length-1);

      gsl_spline_init (xspline[i], tempp, tempx, length);
      gsl_spline_init (yspline[i], tempp, tempy, length);

      /*      if (i==totalnum-1)
	{
	  for (double xi=0;xi<2*M_PI;xi+=0.1)
	    {
	      printf("%f %f %f\n",xi,gsl_spline_eval (xspline[i],xi,xacc[i]),gsl_spline_eval (yspline[i],xi,xacc[i]));
	    }
	}
      */

       //      if (i>switcher)
      //	printf("{%f,%f},\n",taus[i],gsl_spline_eval (yspline[i], 3*M_PI/2., yacc[i]));

      //if ((tempx[j]==0)&&(tempy[j]<0)&&(i>switcher))
	  // printf("{%f,%f},\n",taus[i],tempy[j]);


      /*   
      double yi;
      for (double xi=0;xi<M_PI;xi+=0.1)
        {
	  //yi = gsl_spline_eval (xspline[i], xi, xacc[i]);
	  yi = gsl_spline_eval (uxspline[i], xi, uxacc[i]);
	  //if (i==4000)
	  //{
	      //printf ("%g %g\n", yi,gsl_spline_eval (uxspline[i], 2*M_PI-xi, uxacc[i]));
	      //printf ("%g %g\n", xi,yi);
	      // }
	      if (yi-gsl_spline_eval (uxspline[i], 2*M_PI-xi, uxacc[i])>1.e-2)
		printf ("found at i=%i xi=%f %g %g\n", i,xi,yi,gsl_spline_eval (uxspline[i], 2*M_PI-xi, uxacc[i]));
	      
	}
      */

      delete [] tempx;
      delete [] tempy;
      delete [] tempux;
      delete [] tempuy;
      delete [] temppixx;
      delete [] temppixy;
      delete [] temppiyy;
      delete [] tempp;
    
    }

  //create dtx, dty

  double *tempx,*tempy,*tempp;
  double *datax,*datay;
  tempx=new double[totalnum];
  tempy=new double[totalnum];
  tempp=new double[101];
  datax=new double[101];
  datay=new double[101];

  double bigblockx[100][totalnum];
  double bigblocky[100][totalnum];
  

  gsl_spline * workspline1=gsl_spline_alloc (gsl_interp_cspline, totalnum);
  gsl_interp_accel * workacc1=gsl_interp_accel_alloc ();
  gsl_spline * workspline2=gsl_spline_alloc (gsl_interp_cspline, totalnum);
  gsl_interp_accel * workacc2=gsl_interp_accel_alloc ();
      


  for (int j=0;j<100;j++)
    {
      tempp[j]=2*M_PI/100*j;
      for (int i=0;i<totalnum;i++)
	{
	  tempx[i]=gsl_spline_eval(xspline[i],tempp[j],xacc[i]);
	  tempy[i]=gsl_spline_eval(yspline[i],tempp[j],yacc[i]);
	}
      
      
      gsl_spline_init (workspline1, taus, tempx, totalnum);
      gsl_spline_init (workspline2, taus, tempy, totalnum);
  
      for (int i=0;i<totalnum;i++)
	{
	  bigblockx[j][i]=gsl_spline_eval_deriv(workspline1,taus[i],workacc1);
	  bigblocky[j][i]=gsl_spline_eval_deriv(workspline2,taus[i],workacc2);
	  //bigblockx[j][i]=gsl_spline_eval(workspline1,taus[i],workacc1);
	  //bigblocky[j][i]=gsl_spline_eval(workspline2,taus[i],workacc2);
	}

      //printf("testing %f %f\n",bigblockx[j][0],gsl_spline_eval(xspline[0],tempp[j],xacc[0]));
    }
  
  for (int i=0;i<totalnum;i++)
    {
      for (int j=0;j<100;j++)
	{
	  datax[j]=bigblockx[j][i];
	  datay[j]=bigblocky[j][i];
	}

      datax[100]=datax[0];
      datay[100]=datay[0];
      tempp[100]=2*M_PI;

       
       dtxspline[i] = gsl_spline_alloc (t, 101);      
       dtxacc[i] = gsl_interp_accel_alloc ();
       dtyspline[i] = gsl_spline_alloc (t, 101);      
       dtyacc[i] = gsl_interp_accel_alloc ();


       gsl_spline_init (dtxspline[i], tempp, datax, 101);
       gsl_spline_init (dtyspline[i], tempp, datay, 101);
    }

  //for (int j=0;j<100;j++)
  // {
  ///   printf("t2 %f %f\n",gsl_spline_eval(dtxspline[0],2*M_PI/100*j,dtxacc[0]),gsl_spline_eval(xspline[0],2*M_PI/100*j,xacc[0]));
    //}
  
  gsl_spline_free (workspline1);
  gsl_spline_free (workspline2);

  gsl_interp_accel_free(workacc1);
  gsl_interp_accel_free(workacc2);
    


  delete [] tempx;
  delete [] tempy;
  delete [] tempp; 

}


//gsl_interp_accel * wac=gsl_interp_accel_alloc ();


double rootfunction(double x,void *params)
{
  struct dummy_params *point= (struct dummy_params *) params;

  gsl_spline * ws= point->myspline;
  gsl_interp_accel * wac = point->myacc;

  double temp;
  temp=gsl_spline_eval (ws, x, wac);

  return temp;
}


double gettau(double x,double y)
{
  //  int thisnum=6*(totalnum-switcher)/4;
  //if (totalnum-thisnum<0)
  // {
  //  printf("gettau: out of bounds\n");
  //  exit(-1);
  //}


  int thisnum=totalnum/2;
  
  double datax[thisnum];
  double datay[thisnum];
  double tphi=atan2(y,x);
  if (tphi<0)
    tphi+=2*M_PI;

  double radius2=x*x+y*y;

  double lastr2=gsl_spline_eval (yspline[totalnum-1], tphi, yacc[totalnum-1])*gsl_spline_eval (yspline[totalnum-1], tphi, yacc[totalnum-1]);
  lastr2+=gsl_spline_eval (xspline[totalnum-1], tphi, yacc[totalnum-1])*gsl_spline_eval (xspline[totalnum-1], tphi, yacc[totalnum-1]);

  double maxr2=gsl_spline_eval (yspline[totalnum-thisnum], tphi, yacc[totalnum-thisnum])*gsl_spline_eval (yspline[totalnum-thisnum], tphi, yacc[totalnum-thisnum]);
  maxr2+=gsl_spline_eval (xspline[totalnum-thisnum], tphi, yacc[totalnum-thisnum])*gsl_spline_eval (xspline[totalnum-thisnum], tphi, yacc[totalnum-thisnum]);

  

  //printf("gettau: %f and x=%f y=%f\n",tphi,x,y);
  //printf("c.f minr=%f r=%f maxr=%f\n",lastr2,radius2,maxr2);

  if (lastr2>radius2)
   return -1;
  if (maxr2<radius2)
    return -1;

  gsl_spline * workspline=gsl_spline_alloc (gsl_interp_cspline, thisnum);
  gsl_interp_accel * wac=gsl_interp_accel_alloc ();


  for (int i=0;i<thisnum;i++)
    {
      datax[i]=taus[totalnum-thisnum+i];
      datay[i]=gsl_spline_eval (yspline[totalnum-thisnum+i], tphi, yacc[totalnum-thisnum+i])*gsl_spline_eval (yspline[totalnum-thisnum+i], tphi, yacc[totalnum-thisnum+i]);
      datay[i]+=gsl_spline_eval (xspline[totalnum-thisnum+i], tphi, xacc[totalnum-thisnum+i])*gsl_spline_eval (xspline[totalnum-thisnum+i], tphi, xacc[totalnum-thisnum+i]);
      datay[i]-=radius2;
      //  if (x==0)
      //printf("{%f,%f},\n",datax[i],datay[i]+radius2);
    }

  struct dummy_params dparm;

  gsl_spline_init (workspline, datax, datay, thisnum);
  

  dparm.myspline=workspline;
  dparm.myacc=wac;

  int limiter;
  int status;
  int iter = 0, max_iter = 1000;
  const gsl_root_fsolver_type *TT;
  gsl_root_fsolver *ss;
  double r=0;
  double dummy=0;
  gsl_function F;
 
  

 
  F.function=&rootfunction;
  F.params=&dparm;
      
  TT = gsl_root_fsolver_brent;
  ss = gsl_root_fsolver_alloc (TT);
  
  gsl_root_fsolver_set (ss, &F,taus[totalnum-thisnum],taus[totalnum-1]);
      
      
  iter=0;
  do
    {
      iter++;
      status = gsl_root_fsolver_iterate (ss);
      r = gsl_root_fsolver_root (ss);
      double x_lo = gsl_root_fsolver_x_lower (ss);
      double x_hi = gsl_root_fsolver_x_upper (ss);
      status = gsl_root_test_interval (x_lo, x_hi,(taus[switcher+1]-taus[switcher])/10., 0.001);
    }
  while (status == GSL_CONTINUE && iter < max_iter);
      
  gsl_spline_free (workspline);
  gsl_interp_accel_free(wac);
  gsl_root_fsolver_free (ss);

  

  return r;
}

double getit(double x,double y,double tt,gsl_spline ** ws,gsl_interp_accel ** wsacc)
{

  double tphi=atan2(y,x);
  if (tphi<0)
    tphi+=2*M_PI;

  //int thisnum=6*(totalnum-switcher)/4;
  //if (totalnum-thisnum<0)
  // thisnum=totalnum;

  int thisnum=totalnum;
  double datax[thisnum];
  double datay[thisnum];

  
  gsl_spline * workspline=gsl_spline_alloc (gsl_interp_cspline, thisnum);
  
  for (int i=0;i<thisnum;i++)
    {
      datax[i]=taus[totalnum-thisnum+i];
      datay[i]=gsl_spline_eval (ws[totalnum-thisnum+i], tphi, wsacc[totalnum-thisnum+i]);
      //double hphi=atan2(-y,x);
      //if (probon==1)
      //	printf("inside %f %f\n",datay[i],gsl_spline_eval (ws[totalnum-thisnum+i], hphi, wsacc[totalnum-thisnum+i]));
    }

  gsl_spline_init (workspline, datax, datay, thisnum);
  gsl_interp_accel * wac=gsl_interp_accel_alloc ();

  double temp=gsl_spline_eval (workspline, tt,wac);
  

  gsl_spline_free (workspline);
  gsl_interp_accel_free(wac);

  return temp;
}

void smoothtau(double **grid)
{

  double chisq;
  gsl_matrix *X, *cov;
  gsl_vector *y, *c;

  int mypoly;
  int debug=0;

  for (int i=0;i<numpoints;i++)
    {

      int cntr=0;
      int starter=0;
      for (int j=0;j<numpoints;j++)
	{	  
	  
	  if (grid[i][j]>0)
	    {
	      cntr++;
	      //printf("at %i (total=%i)\n",j,cntr);
	      if (starter==0)
		starter=j;
	    }	  
	}
      
      if (debug==1)
	printf("%i found %i at %i\n",i,cntr,starter);

      if (cntr>2)
	{

	  if (cntr<=POLYORD)
	    mypoly=cntr-1;
	  else
	    mypoly=POLYORD;

	  if (debug==1)
	    printf("mypoly %i\n",mypoly);

	  X=gsl_matrix_alloc (cntr, mypoly);
	  y = gsl_vector_alloc (cntr);

	  c = gsl_vector_alloc (mypoly);
	  cov = gsl_matrix_alloc (mypoly, mypoly);

	  gsl_multifit_linear_workspace * fwork = gsl_multifit_linear_alloc (cntr, mypoly);

	  int ocntr=0;
	  for (int j=0;j<numpoints;j++)
	    {
	      
	      //double xx=(i-middle)*stepper;
	      double yy=(j-middle)*stepper;
	      if (grid[i][j]>0)
		{
		  
		  gsl_matrix_set (X, ocntr, 0, 1.0);
		  for (int k=1;k<mypoly;k++)
		    gsl_matrix_set (X, ocntr, k, pow(yy,k*2));
		  gsl_vector_set (y, ocntr, grid[i][j]);
		  //printf("ocntr %i %f %f\n",ocntr,gsl_vector_get(y,ocntr),gsl_matrix_get(X,ocntr,1));
		  ocntr++;
		}
	    }

	  gsl_multifit_linear (X, y, c, cov,&chisq, fwork);

	  for (int j=starter;j<numpoints-starter;j++)
	    {
	      double yy=(j-middle)*stepper;
	      double temp=gsl_vector_get(c,0);
	      for (int k=1;k<mypoly;k++)
		temp+=gsl_vector_get(c,k)*pow(yy,k*2);
	      
	      //printf("%i old %f new %f\n",i,grid[i][j],temp);
	      grid[i][j]=temp;
	    }

	  gsl_multifit_linear_free (fwork);
	  
	  gsl_matrix_free (X);
	  gsl_vector_free (y);
	  gsl_vector_free (c);
	  gsl_matrix_free (cov);
	}
    }


  for (int i=0;i<numpoints;i++)
    {
      int cntr=0;
      int starter=0;
      for (int j=0;j<numpoints;j++)
	{	  
	  
	  if (grid[j][i]>0)
	    {
	      cntr++;
	      //printf("at %i (total=%i)\n",j,cntr);
	      if (starter==0)
		starter=j;
	    }	  
	}
      
      if (debug==1)
	printf("%i found %i at %i\n",i,cntr,starter);

      if (cntr>2)
	{

	  if (cntr<=POLYORD)
	    mypoly=cntr-1;
	  else
	    mypoly=POLYORD;

	  if (debug==1)
	    printf("mypoly %i\n",mypoly);

	  X=gsl_matrix_alloc (cntr, mypoly);
	  y = gsl_vector_alloc (cntr);

	  c = gsl_vector_alloc (mypoly);
	  cov = gsl_matrix_alloc (mypoly, mypoly);

	  gsl_multifit_linear_workspace * fwork = gsl_multifit_linear_alloc (cntr, mypoly);

	  int ocntr=0;
	  for (int j=0;j<numpoints;j++)
	    {
	      
	      double xx=(j-middle)*stepper;
	      //double yy=(j-middle)*stepper;
	      if (grid[j][i]>0)
		{
		  gsl_matrix_set (X, ocntr, 0, 1.0);
		  for (int k=1;k<mypoly;k++)
		    gsl_matrix_set (X, ocntr, k, pow(xx,k*2));
		  gsl_vector_set (y, ocntr, grid[j][i]);
		  //printf("ocntr %i %f %f\n",ocntr,gsl_vector_get(y,ocntr),gsl_matrix_get(X,ocntr,1));
		  ocntr++;
		}
	    }

	  gsl_multifit_linear (X, y, c, cov,&chisq, fwork);

	  for (int j=starter;j<numpoints-starter;j++)
	    {
	      double xx=(j-middle)*stepper;
	      double temp=gsl_vector_get(c,0);
	      for (int k=1;k<mypoly;k++)
		temp+=gsl_vector_get(c,k)*pow(xx,k*2);
	      
	      //printf("%i old %f new %f\n",i,grid[j][i],temp);
	      grid[j][i]=temp;
	    }

	  gsl_multifit_linear_free (fwork);
	  
	  gsl_matrix_free (X);
	  gsl_vector_free (y);
	  gsl_vector_free (c);
	  gsl_matrix_free (cov);
	}
    }



}



//this we need for correctly
//integrating up the last 10%
void interpolate2()
{
  

  

  //allocate memory

  //double taugrid[numpoints][numpoints];

  double ** taugrid;
  taugrid=new double*[numpoints];
  for (int i=0;i<numpoints;i++)
    taugrid[i]=new double[numpoints];

  //maximum physical size:

  double size=gsl_spline_eval (xspline[switcher], 0, xacc[switcher]);
  if (gsl_spline_eval (yspline[switcher], M_PI/2, yacc[switcher])>size)
    size=gsl_spline_eval (yspline[switcher],M_PI/2 , yacc[switcher]);
  
  printf("size1 %f attained at %f\n",size,taus[switcher]);
  printf("compare %f\n",gsl_spline_eval (xspline[0], 0, xacc[0]));


  

  printf("middle %i\n",middle);


  stepper=2*size/(numpoints-2);

  //increase by a little to account for edges
  //stepper*=1.1;

  for (int i=0;i<numpoints;i++)
    {
    for (int j=0;j<numpoints;j++)
      {
	double xx=(i-middle)*stepper;
	double yy=(j-middle)*stepper;
	if (sqrt(xx*xx+yy*yy)<=size*1.1)
	  {
	  taugrid[i][j]=gettau(xx,yy);
	  //if (i==56)
	  //  printf("getting tau %i %f\n",j,taugrid[i][j]);
	  //if (i==2)
	  //  printf("%i %i %f %f %f %f\n",i,j,taugrid[i][j],xx,yy,size*1.1);
	  //if ((i==15)&&(j<=middle))
	  // {
	  //	printf("j=%i t=%f vs %f\n",j,gettau(xx,yy),gettau(-xx,yy));
	  //  }
	  //if (gettau(xx,yy)-gettau(-xx,yy)>1.e-2)
	  //  printf("problem at %i %i\n",i,j);
	  }
	else
	  taugrid[i][j]=-1;
	//if ((i==80)&&(taugrid[i][j]>0))
	//  printf("here %f %f\n",yy,taugrid[i][j]);
      }
    }

  //smoothtau(taugrid);

  /*
  fstream tttt;

  tttt.open("data/tttt.dat", ios::out);
  
  for (int i=0;i<numpoints;i++)
    { 
      for (int j=0;j<numpoints;j++)
	{
	  //if (taugrid[i][j]>0)
	    {
	      //tttt << (i-middle)*stepper << "\t";
	      //tttt << (j-middle)*stepper << "\t";
	      tttt << taugrid[i][j] << "\t";//<< "\n";
	    }
	}
      tttt << "\n";
    }

  tttt.close();
  */




  //and generate dtau/dx

  double dxtaugrid[numpoints][numpoints];
  int ocntr=0;

  double temptx[numpoints];
  double tempxx[numpoints];

  // double limy=gsl_spline_eval (yspline[switcher], M_PI/2, yacc[switcher]);



  for (int j=0;j<numpoints;j++)
    {
      ocntr=0;
      for (int i=0;i<numpoints;i++)
	{
	  double xx=(i-middle)*stepper;
	  double yy=(j-middle)*stepper;
	  
	  
	  //if (fabs(yy)<limy)
	  if (taugrid[i][j]>0)
	    {
	      temptx[ocntr]=taugrid[i][j];
	      tempxx[ocntr]=xx;
	      ocntr++;
	    }

	}
    
    
      //printf("alive %i %i\n",j,ocntr);
      if (ocntr>2)
	{
	  gsl_spline * workspline1=gsl_spline_alloc (gsl_interp_cspline, ocntr);
	  gsl_interp_accel * workacc1=gsl_interp_accel_alloc ();


	  gsl_spline_init (workspline1, tempxx, temptx, ocntr);
      
	  for (int i=0;i<numpoints;i++)
	    {
	      double xx=(i-middle)*stepper;
	      if (taugrid[i][j]>0)
		{
		  dxtaugrid[i][j]=gsl_spline_eval_deriv(workspline1,xx,workacc1);
		  //printf("generating for %i %f\n",i,dxtaugrid[i][j]);
		}
	      else 
		dxtaugrid[i][j]=0.0;
	    }
      
	  gsl_spline_free (workspline1);
      
	  gsl_interp_accel_free(workacc1);
	}
      else 
	{
	  for (int i=0;i<numpoints;i++)
	    {
	      dxtaugrid[i][j]=0.0;
	    }
	}
    }


  
  /*
  fstream tttt2;

  tttt2.open("data/dxtt.dat", ios::out);
  
  for (int i=0;i<numpoints;i++)
    { 
      for (int j=0;j<numpoints;j++)
	{
	  //if (taugrid[i][j]>0)
	    {
	      //tttt << (i-middle)*stepper << "\t";
	      //tttt << (j-middle)*stepper << "\t";
	      tttt2 << dxtaugrid[i][j] << "\t";//<< "\n";
	    }
	}
      tttt2 << "\n";
    }

  tttt2.close();
  */


  printf("Finished interpolating tau surface\n");
  
  double helpgridx[numpoints],helpgridy[numpoints];

  
  int counter=0;

  //and play again:

  
  const gsl_interp_type *t = gsl_interp_cspline; 
  

  

  for (int i=0;i<numpoints;i++)
    {
      counter=0;
      int starter=0;
      for (int j=0;j<numpoints;j++)
	{
	  if (taugrid[i][j]>0)
	    {
	      if (starter==0)
		starter=j;
	      helpgridx[j]=(j-middle)*stepper;
	      helpgridy[j]=taugrid[i][j];
	      counter++;
	    }
	  else
	    {
	      //printf("%i %i\n",i,j);
	    }
	}

      counterarr[i]=0;
      boundarr[i]=0.0;

      double xx=(i-middle)*stepper;
      //if (fabs(xx)<limiter)
      if (counter>2)
	{
	  //printf("i=%i cntr=%i\n",i,counter);
	  counterarr[i]=counter;
	  double tempux[counter];
	  double tempuy[counter];
	  double temppixx[counter];
	  double temppixy[counter];
	  double temppiyy[counter];
	  double tempdxt[counter];

	  int anocntr=0;
	  //int test=0;
	  //int cheat=0;
	  //printf("have %f c %f\n",gsl_spline_eval(uxspline[4000],0.0,uxacc[4000]),getit(gsl_spline_eval(xspline[4000],0.0,xacc[4000]),0.0,taus[4000],uxspline,uxacc));

	  for (int j=0;j<numpoints;j++)
	    {
	      if (taugrid[i][j]>0)
		{
		  //test++;
		  double yy=(j-middle)*stepper;
		  //if (boundarr[i]==0.0)
		  //  boundarr[i]=-yy;
		  helpgridy[anocntr]=taugrid[i][j];
		  helpgridx[anocntr]=yy;
		  //if (i==middle)
		  //  printf("{%f,%f},\n",helpgridx[j],helpgridy[j]);

		  tempux[anocntr]=getit(xx,yy,helpgridy[anocntr],uxspline,uxacc);
		  tempuy[anocntr]=getit(xx,yy,helpgridy[anocntr],uyspline,uyacc);


		  //if ((i==56)&&(yy>6.68))
		  //if ((i==56))
		  //  printf("%i %i =%f %f %f\n",i,j,yy,tempux[anocntr],tempuy[anocntr]);

		  //printf("in the process %f vs %f at %f %f\n",tempux[j]/tempuy[j],xx/yy,xx,yy);
		  //printf("%i %i %f %f rat=%f vs =%f\n",i-middle,j-middle,tempux[anocntr],tempuy[anocntr],tempux[anocntr]/tempuy[anocntr],(i-middle)/(0.0+j-middle));

		  temppixx[anocntr]=getit(xx,yy,helpgridy[anocntr],pixxspline,pixxacc);
		  temppixy[anocntr]=getit(xx,yy,helpgridy[anocntr],pixyspline,pixyacc);
		  temppiyy[anocntr]=getit(xx,yy,helpgridy[anocntr],piyyspline,piyyacc);
		  tempdxt[anocntr]=dxtaugrid[i][j];
		  //printf("got here %i\n",anocntr);
		  //printf("comp %f %f\n",getit(xx,yy,helpgridy[j],uxspline,uxacc),getit(xx,-yy,helpgridy[j],uxspline,uxacc));
		  //	  if (getit(xx,yy,helpgridy[j],uxspline,uxacc)+getit(-xx,yy,helpgridy[j],uxspline,uxacc)>1.e-2)
		  //   {
		  //      printf("problem at i=%i j=%i %f %f\n",i,j,getit(xx,yy,helpgridy[j],uxspline,uxacc),getit(xx,-yy,helpgridy[j],uxspline,uxacc));
		      //probon=1;
		      //getit(xx,yy,helpgridy[j],uxspline,uxacc);
		      //probon=0;
		      
		  // }
		  //printf("testing i=%i t=%f  x=%f y=%f %f\n",i,helpgridy[j],xx,yy,tempux[j]);
		  //printf("testing t=%f  x=%f y=%f %f\n",helpgridx[j],xx,(j-middle)*stepper,tempux[j]);

		  anocntr++;
		}
	      
	    }

	  //	  for (int j=0;j<numpoints;j++)
	  // printf("%i %f\n",j,helpgridx[j]);

	  //printf("test %i\n",test);
	  tacc[i] = gsl_interp_accel_alloc ();
	  ouxacc[i] = gsl_interp_accel_alloc ();
	  ouyacc[i] = gsl_interp_accel_alloc ();
	  opixxacc[i] = gsl_interp_accel_alloc ();
	  opixyacc[i] = gsl_interp_accel_alloc ();
	  opiyyacc[i] = gsl_interp_accel_alloc ();
	  dxtacc[i] = gsl_interp_accel_alloc ();

	  
	  tspline[i] = gsl_spline_alloc (t, counter);
	  //printf("generate for %i\n",i);
	  ouxspline[i] = gsl_spline_alloc (t, counter);	  
	  ouyspline[i] = gsl_spline_alloc (t, counter);
	  opixxspline[i] = gsl_spline_alloc (t, counter);
	  opixyspline[i] = gsl_spline_alloc (t, counter);
	  opiyyspline[i] = gsl_spline_alloc (t, counter);
	  dxtspline[i] = gsl_spline_alloc (t, counter);
	  
	 

	  gsl_spline_init (tspline[i], helpgridx, helpgridy, counter);

	   

	  //	  if (i==12)
	  // printf("%i cf %f %f\n",counter,0.0,gsl_spline_eval (tspline[i], 0.0, tacc[i]));
	  //if (i==80)
	  //  for (double mx=-4.3;mx<4.3;mx+=0.1)
	  //printf("{%f,%f},\n",mx,gsl_spline_eval (tspline[i], mx, tacc[i]));

	  gsl_spline_init (ouxspline[i], helpgridx, tempux, counter);

	  gsl_spline_init (ouyspline[i], helpgridx, tempuy, counter);
	  gsl_spline_init (opixxspline[i], helpgridx, temppixx, counter);
	  gsl_spline_init (opixyspline[i], helpgridx, temppixy, counter);
	  gsl_spline_init (opiyyspline[i], helpgridx, temppiyy, counter);
	  gsl_spline_init (dxtspline[i], helpgridx, tempdxt, counter);

	  //printf("done\n");

	  //printf("generated for %i t= %f\n",i,gsl_spline_eval (tspline[i], 0.0, tacc[i]));
	  //printf("generated for %i ux= %f\n",i,gsl_spline_eval (ouxspline[i], 0.0, ouxacc[i]));
	}
      else
	{
	  //printf("got thrown out %i because cntr=%i\n",i,counter);
	}
    }

  //printf("just testing %f\n",gsl_spline_eval(tspline[12],0.0,tacc[12]));

  
  int start=numset[switcher-1]-switcher+1;
  int length=length=numset[switcher]-numset[switcher-1];

  double anotherx[length];
  double anothery[length];

  int validl=0;

  for (int j=length-2;j>-1;j--)
    {
      //printf("testing %f %f\n",xp[start+j],yp[start+j]);

      //printf("{%f,%f},\n",xp[start+j],yp[start+j]);
      if (atan2(yp[start+j],xp[start+j])>=0)
	{
	  anotherx[validl]=xp[start+j];
	  anothery[validl]=yp[start+j];
	  //anothery[validl]=0.0;//yp[start+j]*0.0;
	  //printf("%i %f %f\n",validl,anotherx[validl],anothery[validl]);
	  validl++;
	}
    }

  /*
    for (int i=0;i<NM;i++)
    {
      double tphi=M_PI-M_PI/99*i;
      anotherx[i]=gsl_spline_eval(xspline[switcher],tphi,xacc[switcher]);
      anothery[i]=gsl_spline_eval(yspline[switcher],tphi,yacc[switcher]);
      printf("test1 %f %f\n",anotherx[i],anothery[i]);
    }
  */

  gsl_spline * workspline=gsl_spline_alloc (gsl_interp_cspline, validl);
      
  gsl_interp_accel * wwwac=gsl_interp_accel_alloc ();

  gsl_spline_init (workspline, anotherx, anothery, validl);
 
  //printf("done\n");

  //double limiter=gsl_spline_eval (xspline[switcher], 0, xacc[switcher]);
 
  double limiter=gsl_spline_eval (workspline,0,wwwac);
 
  printf("lim %f\n",limiter);

  for (int i=0;i<numpoints;i++)
    {
      double xx=(i-middle)*stepper;
      if (fabs(xx)<limiter)
	{
	  boundarr[i]=gsl_spline_eval (workspline, xx,wwwac);
	  //boundarr[i]=sqrt(limiter*limiter-xx*xx);
	}
      else
	boundarr[i]=0.0;
      //printf("test2 %f %f\n",xx,boundarr[i]);
      //printf("boundarr %i %f\n",i,boundarr[i]);
    }
  
  gsl_spline_free (workspline);
  gsl_interp_accel_free (wwwac);


  fstream tttt;

  tttt.open("data/tttt.dat", ios::out);
  
  for (int i=0;i<numpoints;i++)
    { 
      for (int j=0;j<numpoints;j++)
	{
	  double xx=(i-middle)*stepper;
	  double yy=(j-middle)*stepper;
	  if ((counterarr[i]>0)&&(fabs(yy)<boundarr[i]))
	    tttt << gsl_spline_eval(tspline[i],yy,tacc[i]) << "\t";
	  else
	    tttt << 0 << "\t";
	}
      tttt << "\n";
    }
  tttt.close();

  fstream tttt2;

  tttt2.open("data/dxtt.dat", ios::out);
  
  for (int i=0;i<numpoints;i++)
    { 
      for (int j=0;j<numpoints;j++)
	{
	  double xx=(i-middle)*stepper;
	  double yy=(j-middle)*stepper;
	  if ((counterarr[i]>0)&&(fabs(yy)<boundarr[i]))
	    tttt2 << gsl_spline_eval(dxtspline[i],yy,tacc[i]) << "\t";
	  else
	    tttt2 << 0 << "\t";
	}
      tttt2 << "\n";
    }
  tttt2.close();


  fstream tttt3;

  tttt3.open("data/dytt.dat", ios::out);
  
  for (int i=0;i<numpoints;i++)
    { 
      for (int j=0;j<numpoints;j++)
	{
	  double xx=(i-middle)*stepper;
	  double yy=(j-middle)*stepper;
	  if ((counterarr[i]>0)&&(fabs(yy)<boundarr[i]))
	    tttt3 << gsl_spline_eval_deriv(tspline[i],yy,tacc[i]) << "\t";
	  else
	    tttt3 << 0 << "\t";
	}
      tttt3 << "\n";
    }
  tttt3.close();


  fstream tttt4;

  tttt4.open("data/uxtt.dat", ios::out);
  
  for (int i=0;i<numpoints;i++)
    { 
      for (int j=0;j<numpoints;j++)
	{
	  double xx=(i-middle)*stepper;
	  double yy=(j-middle)*stepper;
	  if ((counterarr[i]>0)&&(fabs(yy)<boundarr[i]))
	    tttt4 << gsl_spline_eval(ouxspline[i],yy,ouxacc[i]) << "\t";
	  else
	    tttt4 << 0 << "\t";
	}
      tttt4 << "\n";
    }
  tttt4.close();

  fstream tttt5;

  tttt5.open("data/uytt.dat", ios::out);
  
  for (int i=0;i<numpoints;i++)
    { 
      for (int j=0;j<numpoints;j++)
	{
	  double xx=(i-middle)*stepper;
	  double yy=(j-middle)*stepper;
	  if ((counterarr[i]>0)&&(fabs(yy)<boundarr[i]))
	    tttt5 << gsl_spline_eval(ouyspline[i],yy,ouyacc[i]) << "\t";
	  else
	    tttt5 << 0 << "\t";
	}
      tttt5 << "\n";
    }
  tttt5.close();

  
  
  for (int i=0;i<numpoints;i++)
    delete [] taugrid[i];





}



double allintegrand(double tphi,void * params)
{
  double *par= (double *) params;
  //cout << "par1= " << par[0] << "\t";
  //cout << "par2= " << par[1] << "\n";
  //par[0]=Temperature
  //par[1]=particle rest-mass
  //par[2]=p_T
  //par[3]=phi_p
  //par[4]=setnumber eqiv tau
  double px=par[2]*cos(par[3]);
  double py=par[2]*sin(par[3]);
  double mt=sqrt(par[2]*par[2]+par[1]*par[1]);
  int thisset=(int) par[4];

  double mux=gsl_spline_eval (uxspline[thisset], tphi, uxacc[thisset]);
  double muy=gsl_spline_eval (uyspline[thisset], tphi, uyacc[thisset]);

  double ut=sqrt(1+mux*mux+muy*muy);

  double dpmx=gsl_spline_eval_deriv (xspline[thisset], tphi, xacc[thisset]);
  double dpmy=gsl_spline_eval_deriv (yspline[thisset], tphi, yacc[thisset]);
  double dtmx,dtmy;
  //nice but slow?
  
  //getdtsplines(thisset,phi);

  dtmx=gsl_spline_eval(dtxspline[thisset],tphi,dtxacc[thisset]);
  dtmy=gsl_spline_eval(dtyspline[thisset],tphi,dtyacc[thisset]);
  
  //printf("hmm %f %f\n",dtmx,gsl_spline_eval(xspline[thisset],tphi,xacc[thisset]));

  //dtmx=(gsl_spline_eval (xspline[thisset+1], tphi, xacc[thisset+1])-gsl_spline_eval (xspline[thisset], tphi, xacc[thisset]))/(taus[thisset+1]-taus[thisset]);
  //dtmy=(gsl_spline_eval (yspline[thisset+1], tphi, yacc[thisset+1])-gsl_spline_eval (yspline[thisset], tphi, yacc[thisset]))/(taus[thisset+1]-taus[thisset]);
  
  //printf("dtmx=%f \t dtmy=%f\t dpmx=%f\t dpmy=%f\n",dtmx,dtmy,dpmx,dpmy);

  double d0k0=gsl_sf_bessel_K0(mt/par[0]*ut);
  double d1k0=gsl_sf_bessel_K1(mt/par[0]*ut);


  double result;

  double temp1,temp2;
  
  //printf("get %f with dtmy=%f and dtmx=%f\n",(dtmx*dpmy-dtmy*dpmx),dtmy/sin(tphi),dtmx/cos(tphi)); //printf("{%f,%f},",tphi,gsl_spline_eval (xspline[thisset], tphi, xacc[thisset]));
  //printf("{%f,%f},",tphi,gsl_spline_eval (xspline[thisset+1], tphi, xacc[thisset+1]));

  //temp=1;
  temp1=(dtmx*dpmy-dtmy*dpmx)*mt;
  temp2=px*dpmy-py*dpmx;
  

  //if eta is non-negligible, take visc effects into account:
  if (ETAOS>0.001)
    {
 
      double mpixx=gsl_spline_eval (pixxspline[thisset], tphi, pixxacc[thisset])/(TF*TF*2);
      double mpixy=gsl_spline_eval (pixyspline[thisset], tphi, pixyacc[thisset])/(TF*TF*2);
      double mpiyy=gsl_spline_eval (piyyspline[thisset], tphi, piyyacc[thisset])/(TF*TF*2);
      
      double vx=mux/ut;
      double vy=muy/ut;
      double mpitt=vx*vx*mpixx+2*vx*vy*mpixy+vy*vy*mpiyy;
      double mpiee=mpixx+mpiyy-mpitt;
      double mpitx=vx*mpixx+vy*mpixy;
      double mpity=vx*mpixy+vy*mpiyy;
      
      double one=1+mt*mt*mpiee+px*px*mpixx+py*py*mpiyy+2*px*py*mpixy;
      double two=-2*mt*(px*mpitx+py*mpity);
      double three=mt*mt*(mpitt-mpiee);
           
      double d2k0=(gsl_sf_bessel_Kn(2,mt/par[0]*ut)+d0k0)/2;
      double d3k0=(gsl_sf_bessel_Kn(3,mt/par[0]*ut)+3*d1k0)/4;


      temp1*=(one*d1k0+two*d2k0+three*d3k0);
      temp2*=(one*d0k0+two*d1k0+three*d2k0);

      result=temp1-temp2;
    }
  else
    {
      temp1*=d1k0;
      temp2*=d0k0;
      //printf("tphi: %f temp1 %f temp2 %f\n",tphi,temp1,temp2);
      //printf("first int %f second int %f\n",firstintegrand(tphi,params),secondintegrand(tphi,params));
      result=temp1-temp2;
    }

  result*=exp((px*mux+py*muy)/par[0]);
  //printf("result %f comp %f\n",result,firstintegrand(tphi,params)-secondintegrand(tphi,params));
  return result;

}



double allintegrand2(double yy,void * params)
{

  
  double *par= (double *) params;
  //cout << "par1= " << par[0] << "\t";
  //cout << "par2= " << par[1] << "\n";
  //par[0]=Temperature
  //par[1]=particle rest-mass
  //par[2]=p_T
  //par[3]=phi_p
  //par[4]=setnumber eqiv tau
  double px=par[2]*cos(par[3]);
  double py=par[2]*sin(par[3]);
  double mt=sqrt(par[2]*par[2]+par[1]*par[1]);
  int thisset=(int) par[4];

  //printf("evaluating for %i\n",thisset);

  double mux=gsl_spline_eval (ouxspline[thisset],yy,ouxacc[thisset]);
  double muy=gsl_spline_eval (ouyspline[thisset],yy,ouyacc[thisset]);


  

  //printf("alli y=%f mux=%f muy=%f\n",yy,mux,muy);

  //printf("alli mux=%f muy=%f vs %f %f\n",mux,muy,(thisset-middle)*stepper/rr,yy/rr);
  //printf("mux/muy=%f vs %f at \n",mux/muy,(thisset-middle)*stepper/yy,(thisset-middle)*stepper,yy);

  double ut=sqrt(1+mux*mux+muy*muy);


  double dymt=gsl_spline_eval_deriv (tspline[thisset], yy, tacc[thisset]);
  //double dxmt=(gsl_spline_eval (tspline[thisset+1], yy, tacc[thisset+1])-gsl_spline_eval (tspline[thisset-1], yy, tacc[thisset-1]))/(2*stepper);
  double dxmt=gsl_spline_eval (dxtspline[thisset], yy, dxtacc[thisset]);


  //printf("alli x=%f y=%f dymt=%f dxmt=%f\n",(thisset-middle)*stepper,yy,dymt,dxmt);

  //if (isnan(dxmt)==1)
  // printf("here at %f %i %f\n",yy,thisset,dxmt);

  double d0k0=gsl_sf_bessel_K0(mt/par[0]*ut);
  double d1k0=gsl_sf_bessel_K1(mt/par[0]*ut);


  //printf("mux=%f vs=%f\n",gsl_spline_eval (ouxspline[thisset],yy,ouxacc[thisset]),gsl_spline_eval (ouxspline[thisset],-yy,ouxacc[thisset]));
  //printf("muy=%f vs=%f\n",gsl_spline_eval (ouyspline[thisset],yy,ouyacc[thisset]),gsl_spline_eval (ouyspline[thisset],-yy,ouyacc[thisset]));


  double result;

  double temp1,temp2;
  

  //temp=1;
  temp1=mt;
  temp2=px*dxmt+py*dymt;

  //temp2=px*(thisset-middle)*stepper+py*yy;


  //printf("alli x=%f y=%f temp2 %f\n",(thisset-middle)*stepper,yy,temp2);

  

  //if eta is non-negligible, take visc effects into account:
  if (ETAOS>0.001)
  //if (ETAOS>1.0)
    {
 
      double mpixx=gsl_spline_eval (opixxspline[thisset], yy, opixxacc[thisset])/(TF*TF*2);
      double mpixy=gsl_spline_eval (opixyspline[thisset], yy, opixyacc[thisset])/(TF*TF*2);
      double mpiyy=gsl_spline_eval (opiyyspline[thisset], yy, opiyyacc[thisset])/(TF*TF*2);
      
      double vx=mux/ut;
      double vy=muy/ut;
      double mpitt=vx*vx*mpixx+2*vx*vy*mpixy+vy*vy*mpiyy;
      double mpiee=mpixx+mpiyy-mpitt;
      double mpitx=vx*mpixx+vy*mpixy;
      double mpity=vx*mpixy+vy*mpiyy;
      
      double one=1+mt*mt*mpiee+px*px*mpixx+py*py*mpiyy+2*px*py*mpixy;
      double two=-2*mt*(px*mpitx+py*mpity);
      double three=mt*mt*(mpitt-mpiee);
           
      double d2k0=(gsl_sf_bessel_Kn(2,mt/par[0]*ut)+d0k0)/2;
      double d3k0=(gsl_sf_bessel_Kn(3,mt/par[0]*ut)+3*d1k0)/4;


      temp1*=(one*d1k0+two*d2k0+three*d3k0);
      temp2*=(one*d0k0+two*d1k0+three*d2k0);

      result=temp1-temp2;
    }
  else
    {
      temp1*=d1k0;
      temp2*=d0k0;
      //printf("tphi: %f temp1 %f temp2 %f\n",tphi,temp1,temp2);
      //printf("first int %f second int %f\n",firstintegrand(tphi,params),secondintegrand(tphi,params));
      result=temp1-temp2;
    }

  result*=exp((px*mux+py*muy)/par[0]);
  //result*=exp((px*(thisset-middle)*stepper+py*yy)/par[0]/10.0);
  result*=gsl_spline_eval (tspline[thisset], yy, tacc[thisset]);
  //printf("result %f comp %f\n",result,firstintegrand(tphi,params)-secondintegrand(tphi,params));

  if(isnan(result)!=0)
    {
      printf("problem alli set=%i and yy=%f\n",thisset,yy);
      printf("have %f %f %f\n",gsl_spline_eval (tspline[thisset], yy, tacc[thisset]),exp((px*mux+py*muy)/par[0]),temp1-temp2);
      printf("aha %f %f %f %f %f %f\n",par[0],px*mux+py*muy,px,py,mux,muy);
      printf("cure: %.12g to %.12Lg\n",exp((px*mux+py*muy)/par[0]),expl((px*mux+py*muy)/par[0]));

      printf("boundary %f\n",boundarr[thisset]);
      
    }


  if(isnan(gsl_spline_eval (tspline[thisset], yy, tacc[thisset]))!=0)
    printf("here is the culprit\n");

  return (-result);

}


/*
double gettau(double x,double y,int * faster,double * frac, double *deltau, double * helpphi)
{
  int thisset=totalnum-1;
  double tphi=atan2(x,y);
  while(gsl_spline_eval (xspline[thisset], tphi, xacc[thisset])<x)
    {
      thisset--;
      if (thisset<switcher-1)
	{
	  printf("Error in trying to determine tau\n");
	  break;
	}
    }

  *faster=thisset;
  
  double temp,delx;
  double lx=gsl_spline_eval (xspline[thisset], tphi, xacc[thisset]);

  *deltau=taus[thisset+1]-temp;

  if (thisset<totalnum-1)
    {
      temp=taus[thisset];
      delx=gsl_spline_eval (xspline[thisset+1], tphi, xacc[thisset+1]);
      delx-=lx;
      }
  else
    {
      temp=taus[thisset];
      delx=-gsl_spline_eval (xspline[thisset-1], tphi, xacc[thisset-1]);
      delx+=lx;
    }

  temp+=*deltau/delx*(x-lx);
  *frac=temp-taus[thisset];

  return temp;
}
*/

 /*
double getux(double tphi, int thisset, double deltau, double frac)
{
  double temp=gsl_spline_eval (uxspline[thisset], tphi, uxacc[thisset]);
  double delx;

  if (thisset<totalnum-1)
    delx=gsl_spline_eval (uxspline[thisset+1], tphi, uxacc[thisset+1])-temp;
  else
    delx=temp-gsl_spline_eval (uxspline[thisset-1], tphi, uxacc[thisset-1]);
    
  temp+=delx/deltau*frac;
  return temp;
}


double getuy(double tphi, int thisset, double deltau, double frac)
{
  double temp=gsl_spline_eval (uyspline[thisset], tphi, uyacc[thisset]);
  double delx;

  if (thisset<totalnum-1)
    delx=gsl_spline_eval (uyspline[thisset+1], tphi, uyacc[thisset+1])-temp;
  else
    delx=temp-gsl_spline_eval (uyspline[thisset-1], tphi, uyacc[thisset-1]);
    
  temp+=delx/deltau*frac;
  return temp;
}



double allintegrand2(double y,void * params)
{
  double *par= (double *) params;
  //cout << "par1= " << par[0] << "\t";
  //cout << "par2= " << par[1] << "\n";
  //par[0]=Temperature
  //par[1]=particle rest-mass
  //par[2]=p_T
  //par[3]=phi_p
  //par[4]=x
  double px=par[2]*cos(par[3]);
  double py=par[2]*sin(par[3]);
  double mt=sqrt(par[2]*par[2]+par[1]*par[1]);

  int faster=0;
  double frac=0;
  double helpphi=0;
  double deltau=0;

  double tau=gettau(par[4],y,&faster,&frac,&deltau,&helpphi);

  //int thisset=(int) par[4];

  double mux=getux(helpphi,faster,deltau,frac);
  double muy=getuy(helpphi,faster,deltau,frac);

  double ut=sqrt(1+mux*mux+muy*muy);



  //double dpmx=gsl_spline_eval_deriv (xspline[thisset], tphi, xacc[thisset]);
  //double dpmy=gsl_spline_eval_deriv (yspline[thisset], tphi, yacc[thisset]);
  //double dtmx,dtmy;
  //nice but slow?
  
  //getdtsplines(thisset,phi);

  //dtmx=gsl_spline_eval_deriv(dtxspline,taus[thisset],dtxacc);
  //dtmy=gsl_spline_eval_deriv(dtyspline,taus[thisset],dtyacc);
  
  //dtmx=(gsl_spline_eval (xspline[thisset+1], tphi, xacc[thisset+1])-gsl_spline_eval (xspline[thisset-1], tphi, xacc[thisset-1]))/(taus[thisset+1]-taus[thisset-1]);
  //dtmy=(gsl_spline_eval (yspline[thisset+1], tphi, yacc[thisset+1])-gsl_spline_eval (yspline[thisset-1], tphi, yacc[thisset-1]))/(taus[thisset+1]-taus[thisset-1]);
  
  //printf("dtmx=%f \t dtmy=%f\t dpmx=%f\t dpmy=%f\n",dtmx,dtmy,dpmx,dpmy);

  double d0k0=gsl_sf_bessel_K0(mt/par[0]*ut);
  double d1k0=gsl_sf_bessel_K1(mt/par[0]*ut);


  double result;

  double temp1,temp2;
  
  //printf("get %f with dtmy=%f and dtmx=%f\n",(dtmx*dpmy-dtmy*dpmx),dtmy/sin(tphi),dtmx/cos(tphi));
  //printf("{%f,%f},",tphi,gsl_spline_eval (xspline[thisset], tphi, xacc[thisset]));
  //printf("{%f,%f},",tphi,gsl_spline_eval (xspline[thisset+1], tphi, xacc[thisset+1]));

  //temp=1;
  temp1=mt;
  temp2=px*dpmy-py*dpmx;
  

  //if eta is non-negligible, take visc effects into account:
  if (ETAOS>0.001)
    {
 
      double mpixx=gsl_spline_eval (pixxspline[thisset], tphi, pixxacc[thisset])/(TF*TF*2);
      double mpixy=gsl_spline_eval (pixyspline[thisset], tphi, pixyacc[thisset])/(TF*TF*2);
      double mpiyy=gsl_spline_eval (piyyspline[thisset], tphi, piyyacc[thisset])/(TF*TF*2);
      
      double vx=mux/ut;
      double vy=muy/ut;
      double mpitt=vx*vx*mpixx+2*vx*vy*mpixy+vy*vy*mpiyy;
      double mpiee=mpixx+mpiyy-mpitt;
      double mpitx=vx*mpixx+vy*mpixy;
      double mpity=vx*mpixy+vy*mpiyy;
      
      double one=1+mt*mt*mpiee+px*px*mpixx+py*py*mpiyy+2*px*py*mpixy;
      double two=-2*mt*(px*mpitx+py*mpity);
      double three=mt*mt*(mpitt-mpiee);
           
      double d2k0=(gsl_sf_bessel_Kn(2,mt/par[0]*ut)+d0k0)/2;
      double d3k0=(gsl_sf_bessel_Kn(3,mt/par[0]*ut)+3*d1k0)/4;


      temp1*=(one*d1k0+two*d2k0+three*d3k0);
      temp2*=(one*d0k0+two*d1k0+three*d2k0);

      result=temp1-temp2;
    }
  else
    {
      temp1*=d1k0;
      temp2*=d0k0;
      //printf("tphi: %f temp1 %f temp2 %f\n",tphi,temp1,temp2);
      //printf("first int %f second int %f\n",firstintegrand(tphi,params),secondintegrand(tphi,params));
      result=temp1-temp2;
    }

  result*=exp((px*mux+py*muy)/par[0]);
  //printf("result %f comp %f\n",result,firstintegrand(tphi,params)-secondintegrand(tphi,params));
  return result;

}

 */

double ointegrate1(double T,double m0,double pt, double phip, int thisset)
{
  double result,error;
  size_t neval;
  double parameters[5];

  

  parameters[0]=T;
  parameters[1]=m0;
  parameters[2]=pt;
  parameters[3]=phip;
  parameters[4]=(double) thisset;

  gsl_function F;
  F.function = &allintegrand;
  F.params = &parameters;
  
  

  gsl_error_handler_t * new_handler;

  new_handler=gsl_set_error_handler_off();
    

  int bad=3;

  

  int code=gsl_integration_qag(&F,0,2*M_PI,1e-10,INTACC,INTSPACE,3,w,&result,&error);


  while(code==GSL_EROUND)
    {
      bad++;
      //printf("Roundoff error, badness %i, set %i\n",bad-1,thisset);
      code=gsl_integration_qag(&F,0,2*M_PI,1e-10,INTACC,INTSPACE,bad,w,&result,&error);
      
      if (bad==7)
	{
	  printf("I1: Unrecoverable roundoff error detected. Aborting\n");
	  printf("result=%.12g error=%.12g pt=%f, phip=%f set=%i\n",result,error,pt,phip,thisset);
	  exit(1);
	}
    }


  /* restore original handler */
  gsl_set_error_handler (new_handler);

  /*
  F.function = &firstintegrand;
  F.params = &parameters;
  double temp;
  gsl_integration_qag(&F,0,2*M_PI,1e-10,1e-4,2000,3,w,&temp,&error);
  F.function = &secondintegrand;
  F.params = &parameters;

  gsl_integration_qag(&F,0,2*M_PI,error,1e-4,2000,3,w,&result,&error);
  */
  //printf("comp %f\n",temp-result);
  
  result*=taus[thisset];

  return result;

}

double ointegrate2(double T,double m0,double pt, double phip, int thisset)
{

  
  double result,error;
  size_t neval;
  double parameters[5];

  //printf("got here\n");

  parameters[0]=T;
  parameters[1]=m0;
  parameters[2]=pt;
  parameters[3]=phip;
  parameters[4]=(double) thisset;

  gsl_function F;
  F.function = &allintegrand2;
  F.params = &parameters;
  

  /*  if ((pt>1.01)&&(pt<1.05)&&(thisset==switcher))
    {
      printf("here\n");
      for (int i=0;i<20;i++)
	{
	  parameters[3]=2*M_PI/20*i;
	  gsl_integration_qag(&F,0,2*M_PI,1e-10,1e-2,2000,7,w,&result,&error);
	  printf("phi=%f int=%f\n",parameters[3],result);
	}
	}*/


  gsl_error_handler_t * new_handler;

  new_handler=gsl_set_error_handler_off();
    

  int bad=3;

  double low=-boundarr[thisset];
  double high=boundarr[thisset];

  //printf("low %f high %f\n",low,high);

  int code=gsl_integration_qag(&F,low,high,1e-10,INTACC,INTSPACE,3,w,&result,&error);

  while(code==GSL_EROUND)
    {
      bad++;
      //printf("Roundoff error, badness %i, set %i\n",bad-1,thisset);
      code=gsl_integration_qag(&F,low,high,1e-10,INTACC,INTSPACE,bad,w,&result,&error);
      
      if (bad==7)
	{
	  printf("I2: Unrecoverable roundoff error detected. Aborting\n");
	  printf("result=%.12g error=%.12g pt=%f, phip=%f set=%i\n",result,error,pt,phip,thisset);
	  exit(1);
	}
    }


  /* restore original handler */
  gsl_set_error_handler (new_handler);

  if (isnan(result)!=0)
    printf("Problem here %i\n",thisset);

  return result;

}


double prepareint(double T,double m0,double pt, double phip)
{
  

  double temp;
  double *tarr;
  tarr = new double[switcher];
  //double tarrx[switcher-1];
  for (int i=0;i<switcher;i++)
    {
      //tarr[i]=integrate1(T,m0,pt,phip,i);
      //tarr[i-1]=ointegrate1(T,m0,pt,phip,i);
      tarr[i]=ointegrate1(T,m0,pt,phip,i);
      //tarrx[i-1]=taus[i];
      //printf("%i %i %f %f\n",i,switcher,taus[i],tarr[i]);
      //printf("{%f,%f},\n",taus[i],tarr[i]);
      //printf("t=%f \t tarr = %f\n",taus[i],tarr[i]);
      //printf("comp to %f\n",ointegrate1(T,m0,pt,phip,i));
    }
  
  //printf("last should be at %i %f\n",switcher-1,taus[switcher-1]);

  //gsl_spline_init (tsspline,tarrx,tarr,switcher-1);
  gsl_spline_init (tsspline,taus,tarr,switcher-1);




  temp=gsl_spline_eval_integ (tsspline,taus[0],taus[switcher-1] ,tsacc);

  //printf("done1: %f\n",temp);
  //determine number of points and offset
  int starter=0;
  int restpoints=0;

  double limiter=gsl_spline_eval (xspline[switcher], 0, xacc[switcher]);

  for (int i=0;i<numpoints;i++)
    {
      double xx=(i-middle)*stepper;
      if(fabs(xx)<limiter)
	{
	  restpoints++;
	  if (starter==0)
	    starter=i;
	}
    }


  double *tarr2;
  double *xarr2;
  tarr2 = new double[restpoints];
  xarr2 = new double[restpoints];

  //printf("pt=%f phip=%f switching at %i t=%f, restpoints=%i\n",pt,phip,switcher,taus[switcher],restpoints);

  int offset=starter;
  for (int i=0;i<numpoints;i++)
    {
      double xx=(i-middle)*stepper;
      if (fabs(xx)<limiter)
	{
	  if (counterarr[i]>2)
	    {
	      //printf("hmmm %i\n",i-starter);
	      xarr2[i-starter]=(i-middle)*stepper;
	      tarr2[i-starter]=ointegrate2(T,m0,pt,phip,i);
	    }
	  else
	    {
	      xarr2[i-starter]=(i-middle)*stepper;
	      tarr2[i-starter]=0.0;
	    }
	}
      //printf("%i %i %f %f\n",i-starter,restpoints,xarr2[i-starter],tarr2[i-starter]);
      //printf("{%f,%f},\n",xarr2[i-starter],tarr2[i-starter]);
      
    }

  gsl_spline * tsspline2=gsl_spline_alloc (gsl_interp_cspline, restpoints);
  gsl_interp_accel * tsacc2=gsl_interp_accel_alloc ();

  gsl_spline_init (tsspline2,xarr2,tarr2,restpoints);

  temp+=gsl_spline_eval_integ (tsspline2,-limiter,limiter,tsacc2);

  if (temp>0)
    printf("negative result d1=%.12g d2=%.12g at pt=%f phi=%f\n",temp-gsl_spline_eval_integ (tsspline2,-limiter,limiter,tsacc2),gsl_spline_eval_integ (tsspline2,-limiter,limiter,tsacc2),pt,phip);

  //printf("done2 %f, total %f\n",gsl_spline_eval_integ (tsspline2,-limiter,limiter,tsacc2),temp);

  if (isnan(gsl_spline_eval_integ (tsspline2,-limiter,limiter,tsacc2))!=0)
    {
      for (int i=0;i<restpoints;i++)
	{
	  printf("%i %f %f\n",i,xarr2[i],tarr2[i]);
	}
    }
      //printf("%i %i %f %f\n",i-starter,restpoints,xarr2[i-starter],tarr2[i-starter]);
      //printf("{%f,%f},\n",xarr2[i-starter],tarr2[i-starter]);
      


  //gsl_spline_init (tsspline,taus,tarr,switcher-1);

  
  temp/=2*M_PI;
  temp/=2*M_PI;
  temp/=2*M_PI;
  temp*=2;
  temp*=fmtoGeV;
  temp*=fmtoGeV;
  temp*=-fmtoGeV;

  

  delete [] tarr;
  delete [] tarr2;
  delete [] xarr2;

  gsl_spline_free(tsspline2);
  gsl_interp_accel_free(tsacc2);

  
  return temp;
}

double dummyshell(double tau,void * params)
{
  double temp;
  temp=gsl_spline_eval(tsspline,tau,tsacc);
  return temp;
}

/*
double doint()
{
  double dum=1.0;
  size_t neval;

  double result,error;

  gsl_function F;
  F.function = &dummyshell;
  F.params = &dum;
  
  gsl_integration_qag(&F,taus[0],taus[switcher-1],1e-10,1e-3,2000,3,w,&result,&error);

  printf("result %f\t error %f\n",result,error);

  result/=2*M_PI;
  result/=2*M_PI;
  result/=2*M_PI;
  result*=2;
  result*=fmtoGeV;
  result*=fmtoGeV;
  result*=-fmtoGeV;

  return result;
}
*/

void testing()
{

  double result1,error1;
  double result2,error2;
  size_t neval;
  double parameters[5];

  double T=0.15;
  double m0=0.13957;
  double pt=0.1;
  int thisset=1;

   parameters[0]=T;
   parameters[1]=m0;
   parameters[2]=pt;
   parameters[4]=(double) thisset;
   parameters[3]=0.1;

   //printf("why %f\n",firstintegrand(0.5,parameters));

   double mux;
   double muy;

   double mx,nx;
   double my,ny;
   int jk=0;
   double rr=0;
   double nrr=0;
   double ut=0;
   double ur=0;
   double drdt=0;

   for (double phip=0;phip<2*M_PI;phip+=0.1)
    {
      jk++;
      mx=gsl_spline_eval (xspline[thisset], phip, xacc[thisset]);
      my=gsl_spline_eval (yspline[thisset], phip, yacc[thisset]);
      nx=gsl_spline_eval (xspline[thisset+1], phip, xacc[thisset+1]);
      ny=gsl_spline_eval (yspline[thisset+1], phip, yacc[thisset+1]);
      mux=gsl_spline_eval (uxspline[thisset], phip, uxacc[thisset]);
      muy=gsl_spline_eval (uyspline[thisset], phip, uyacc[thisset]);
      //printf("rr= %f\n",sqrt(mx*mx+my*my));
      rr+=sqrt(mx*mx+my*my);
      nrr+=sqrt(nx*nx+ny*ny);
      ut+=sqrt(1+mux*mux+muy*muy);
      ur+=(mx*mux+my*muy);
      drdt+=(sqrt(nx*nx+ny*ny)-sqrt(mx*mx+my*my))/(taus[thisset+1]-taus[thisset]);
    }

   
   nrr/=jk;
   rr/=jk;
   drdt/=jk;
   ut/=jk;
   ur/=jk*rr;

   double mt=sqrt(pt*pt+m0*m0);
   //double ut=sqrt(1+mux*mux+muy*muy);
   //double rr=sqrt(mx*mx+my*my);
   //double ur=(mx*mux+my*muy)/rr;

   printf("Mean r=%f \t at t=%f\n",rr,taus[thisset]);
   printf("Mean drdt=%f ur=%f,ut=%f\n",drdt,ur,ut);
   //printf("Mean nr=%f \t at t=%f\n",nrr,taus[thisset+1]);
   /*
   
   for (double phip=0;phip<2*M_PI;phip+=0.1)
    {
      mx=gsl_spline_eval (xspline[thisset], phip, xacc[thisset]);
      my=gsl_spline_eval (yspline[thisset], phip, yacc[thisset]);
      double dpmx=gsl_spline_eval_deriv (xspline[thisset], phip, xacc[thisset]);
      double dpmy=gsl_spline_eval_deriv (yspline[thisset], phip, yacc[thisset]);
      firstintegrand(phip,parameters);
      printf("drdt %f with %f\n",rr*drdt,drdt);
      //printf("{%f,%f,%f},",phip,my,rr*sin(phip));
      //printf("{%f,%f,%f},",phip,dpmx,-rr*sin(phip));
      //printf("{%f,%f,%f}\n",phip,dpmx,-rr*sin(phip));
      //printf("{%f,%f,%f}\n",phip,dpmy,rr*cos(phip));
    }
   cout << endl;
   */

   double tt1,tt2;
   tt1=2*M_PI*drdt*rr*mt*gsl_sf_bessel_K1(mt/T*ut)*gsl_sf_bessel_I0(pt/T*ur);
   //tt1=2*M_PI*gsl_sf_bessel_K1(mt/T*ut)*gsl_sf_bessel_I0(pt/T*ur);
   tt2=2*M_PI*pt*rr*gsl_sf_bessel_K0(mt/T*ut)*gsl_sf_bessel_I1(pt/T*ur);
   //tt=2*M_PI*gsl_sf_bessel_I0(pt/T*ur);
    //tt2=gsl_sf_bessel_K0(mt/T*ut);

   
   /*
  for (double phip=0;phip<2*M_PI;phip+=0.1)
    {
     
      parameters[3]=phip;
      

      gsl_function F1,F2;
      F1.function = &firstintegrand;
      F1.params = &parameters;
  
      F2.function = &secondintegrand;
      F2.params = &parameters;

      gsl_integration_qag(&F1,0,2*M_PI,1e-10,1e-3,2000,3,w,&result1,&error1);
      gsl_integration_qag(&F2,0,2*M_PI,1e-3,1e-3,2000,3,w,&result2,&error2);
       //double temp1=integrate1(0.15,0.13957,0.1,phip,5);
      //printf("phip=%f t1= %f\t t2=%f \n",phip,temp1);
      printf("phip=%f \t res1 = %f\t res2=%f\n",phip,result1,result2);
    }
   */
  printf("comto\t res1=%f \t res2=%f\n",tt1,tt2);
   
  tt1/=2*M_PI;
  tt1/=2*M_PI;
  tt1/=2*M_PI;
  tt1*=2;
  tt1*=fmtoGeV;
  tt1*=fmtoGeV;
  tt1*=-fmtoGeV;

  tt2/=2*M_PI;
  tt2/=2*M_PI;
  tt2/=2*M_PI;
  tt2*=2;
  tt2*=fmtoGeV;
  tt2*=fmtoGeV;
  tt2*=-fmtoGeV;
  printf("normed\t res1=%f \t res2=%f\n",tt1,tt2);

   /*
    for (double phip=0;phip<2*M_PI;phip+=0.1)
    {
      mx=gsl_spline_eval (xspline[thisset], phip, xacc[thisset]);
      my=gsl_spline_eval (yspline[thisset], phip, yacc[thisset]);

      double r=sqrt(mx*mx+my*my);
      printf("x =%f \t vs. rcos %f\n",gsl_spline_eval (xspline[thisset], phip, xacc[thisset]),rr*cos(phip));
      //printf("r =%f \t vs. rr %f\n",r,rr);
    }
   */
}

void generatetab()
{
  double gsfact=1;
  double tempmass=0;
  double oldmass=0;
  char buffer[maxline];
  int i=0;
  double resbuff[PTASIZE][4*PHIPASIZE]; //length should match length of pt*phi array!
  double ptbuff[PTASIZE];
//   double phipbuff[PHIPASIZE];
  double fullphipbuff[4*PHIPASIZE];

  for (int k=0;k<4*PHIPASIZE;k++) fullphipbuff[k] = k*M_PI/2/PHIPASIZE;

int test = 0;
int particle = 0;
double PHIPMAX = PHIPASIZE;
if(FULL) PHIPMAX = 4*PHIPASIZE;
  while (!massfile.eof())
    {
      massfile >> tempmass;
      gsfile >> gsfact;
      namesfile.getline(buffer,maxline,'\n');
      double cplist[ntemp];
      if (PCE) 
      {
	for (int t=0; t<ntemp; t++)
	{
	  cplist[t] = chem[particle][t];
	}     
      gsl_spline_init (chemspline,Tii,cplist,ntemp);
      }
//       cout << endl << endl << gsl_spline_eval(chemspline,.112,chemacc) << endl << endl;

      //printf("buffer0: %i\n",(int) buffer[0]);
  
      int carret=0;
      if (((int)buffer[0])!=carret)
	{
	  printf("Generating table for %s with mass %f and spin gs=%f\n",buffer,tempmass,gsfact);
	  if (tempmass!=oldmass)
	    {
	      int j=0;
	      //phip-table
	      for (double pt=0.01;pt<PTMAX;pt+=PTMAX/PTASIZE)
		{
		  for(int k=0;k<PHIPMAX;k++)
		    {
		      //prepareint(TF,tempmass,pt,phipbuff[k]);
		      
		      ptbuff[j]=pt;
		      //resbuff[j][k]=doint();
		      //printf("pt = %f phi =% f\n",pt,phipbuff[k]);
		      //get integral times spin degeneracy factor
		      if (FREEZE < 2) resbuff[j][k]=gsfact*prepareint(TF,tempmass,pt,fullphipbuff[k]);
		      else resbuff[j][k]=gsfact*blockintegrate(particle, tempmass,pt,fullphipbuff[k]);

		      if (isnan(resbuff[j][k])!=0)
			  printf("Problem at %f %f\n",pt,fullphipbuff[k]);

		      //printf("result =%f\n",resbuff[j][k]);
		      //printf("pt = %f phi =% f res=%f\n",pt,phipbuff[k],resbuff[j][k]);
		    }
		  j++;
		}
	    }
	  else
	    {
	      //don't do anything, just repeat last result
	    }
	  
//	  for (int k=0;k<PHIPASIZE;k++)
//	    {
//	      for (int j=0;j<PTASIZE;j++)
//		{
//		  //pttab << ptbuff[j] << "\t";
//		  double out = .25*(resbuff[j][k] + resbuff[j][4*PHIPASIZE-k-1] + resbuff[j][2*PHIPASIZE-k-1] + resbuff[j][2*PHIPASIZE+k]);
//		  pttab << out << "\t";
//		}
//	      pttab << "\n";
//	    }
//
//
//
	  for (int k=0;k<4*PHIPASIZE;k++)
	    {
		    if (test == 1)
		    {
			    testfile << fullphipbuff[k] << "\t" << resbuff[PTASIZE/2][k] << endl;
		    }

	      for (int j=0;j<PTASIZE;j++)
		{
		  //pttab << ptbuff[j] << "\t";
		  double out = resbuff[j][k];
		  pttab << out << "\t";
		}
	      pttab << "\n";
	    }




	  oldmass=tempmass;
      
// 	  cout << "i = " << i << endl;
      
	  i++;
	  
// 	  cout << "i = " << i << endl;
	  
// 	  if (i == 1 | i == 319) {
// 	  
// 	    cout << "Min T2 = " << twomin << endl;
//   cout << "Max T2 = " << twomax << endl;
//   cout << "Min T3 = " << threemin << endl;
//   cout << "Max T3 = " << threemax << endl; }
	  
	}
      test++;
      particle++;
    }

  printf("Finished main loop\n");

  //generate pt-table:
  
  ptfile << PTASIZE << endl;
  ptfile << PHIPASIZE << endl;
  for (int j=0;j<PTASIZE;j++)
    {
      ptfile << ptbuff[j] << "\n";
    }

  printf("Done!\n");
  
}

void singlept(double mass)
{
  
  double tempmass=mass;
  double oldmass=0;
  char buffer[maxline];
  int i=0;
  double resbuff[PTASIZE][PHIPASIZE]; //length should match length of pt*phi array!
  double ptbuff[PTASIZE];
  
  int j=0;
  //phip-table
  for (double pt=0.01;pt<PTMAX;pt+=PTMAX/PTASIZE)
    {
      int k=0;
      for (double phip=0;phip<M_PI;phip+=M_PI/PHIPASIZE)
	{
	  
	  prepareint(TF,tempmass,pt,PHIPASIZE);
	  
	  ptbuff[j]=pt;
	  //resbuff[j][k]=doint();
	  resbuff[j][k]=prepareint(TF,tempmass,pt,phip);
	  
	  printf("pt=%f phip=%f result =%f\n",ptbuff[j],phip,resbuff[j][k]);
	  k++;
	}
      j++;
    }

    
  for (int k=0;k<PHIPASIZE;k++)
    {
      for (int j=0;j<PTASIZE;j++)
	{
	  //pttab << ptbuff[j] << "\t";
	  pttab << resbuff[j][k] << "\t";
	}
      pttab << "\n";
    }
  
  
  //generate pt-table:
  
  ptfile << PTASIZE << endl;
  ptfile << PHIPASIZE << endl;
  for (int j=0;j<PTASIZE;j++)
    {
      ptfile << ptbuff[j] << "\n";
    }
  
  printf("Done!\n");
  
}




int main (void)
{

  extern void readParameters(const char*);

  readParameters("data/params.txt");
  
  //open data file

  freeze_out.open("data/freezeout.dat", ios::in);

  countsets();
  
  freeze_out.close();

  loadeos();

  if (FREEZE < 2)
  {

    allocMem();

    cout << "total number of tau's " << totalnum << endl;

    dummy.open("data/freezeout.dat", ios::in);

    readsets();

    dummy.close();

    interpolate1();

    printf("First interpolation finished\n");
    interpolate2();
    printf("Second interpolation finished\n");

  }
  
  else 
  {
    blockallocMem();
    dummy.open("data/freezeout.dat", ios::in);
    blockreadsets();
    dummy.close();
  }
  
  massfile.open("input/pasim.dat", ios::in);
  namesfile.open("input/pasinames.dat", ios::in);
  gsfile.open("input/gslist.dat",ios::in);

  pttab.open("data/phipspectra.dat", ios::out);
  ptfile.open("data/ptarr.dat", ios::out);
  testfile.open("data/testfile.dat", ios::out);

  generatetab();
  
  testfile.close();
  massfile.close();
  namesfile.close();
  gsfile.close();


  if (FREEZE < 2) freeMem();

  ptfile.close();
  pttab.close();

  return 0;
}
