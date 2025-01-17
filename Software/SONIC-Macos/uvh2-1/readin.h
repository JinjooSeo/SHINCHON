#define NUMDECAY        1700 /* size of array for storage of the decays */
#define NUMPARTICLE     400 /*  size of array for storage of the particles */
#define MAXINTV         20300000 /* size of arry for Montecarlo numbers */
#define MHALF           (MAXINTV/2)
#define NPT             200 /* size of arry for storage of the pt-spectrum */
#define NPHI            50 /* size of arry for storage of the y-spectrum */
#define NPHI1           NPHI + 1 
#define PI              3.14159265358979323 /* any question? */
#define ALPHA           0.00729735308 /* fine structure constant 1/137.035...*/
#define HBARC           0.197327054 /* = plank constant times speed of light */
#define HBARC3          (HBARC*HBARC*HBARC) 
#define FILEDIM         140

/* parameter array for each particle and resonance */
struct par  {
        long int        monval;     /* Montecarlo number according PDG */
        char    name[26];
        double  mass;
        double  width;
        int     gspin;      /* spin degeneracy */
        int     baryon;
        int     strange;
        int     charm;
        int     bottom;
        int     gisospin;  /* isospin degeneracy */
        int     charge;
        int     decays;    /* amount of decays listed for this resonance */
        int     stable;     /* defines whether this particle is considered
                                as stable */
        int     npt;
        int     nphi;
        double  dNdptdphi[NPT][NPHI];
        double  mt[NPT];        /* mt values for spectrum */
        double  pt[NPT];        /* pt values for spectrum */
        double  slope[NPHI1];           /* assymtotic slope of mt-spectrum */
        } particle[NUMPARTICLE];

/* decay array for each decay listed */
struct de  {
        int     reso;       /* Montecarlo number of decaying resonance */
        int     numpart;    /* number of daughter particles after decay */
        double  branch;     /* branching ratio */
        int     part[5];    /* array of daughter particels Montecarlo number */
        }  decay[NUMPARTICLE][100];
	
/* array for converting Montecarlo numbers in internal numbering of the 
resonances */
long int        partid[MAXINTV];
int 		stablelookup[319];
int 		stablelookuprev[30];
