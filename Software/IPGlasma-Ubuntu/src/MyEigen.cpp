// MyEigen.cpp is part of the IP-Glasma solver.
// Copyright (C) 2012 Bjoern Schenke.
#include "MyEigen.h"

//**************************************************************************
// MyEigen class.

void MyEigen::flowVelocity4D(Lattice *lat, Group *group, Parameters *param, int it)
{
  int N = param->getSize();
  int pos;
  int changeSign;
  int count;
  double L = param->getL();
  double a = L/N; // lattice spacing in fm
  double x, y, ux=0., uy=0., ueta=0., utau=0.;
  double dtau = param->getdtau();
  gsl_complex square;
  gsl_complex factor;
  gsl_complex euklidiansquare;
  double eps;
  gsl_complex z_aux;
  gsl_complex tau2;
  int foundU;
  double averageux=0.;
  double averageuy=0.;
  double averageueta=0.;
  double averageeps=0.;
  count = 0;
  double uxd, uyd, uetad;
  
  for (int si=0; si<N; si++)
    {
      for (int sj=0; sj<N; sj++)
	{

	  x = -L/2.+a*si;
	  y = -L/2.+a*sj;
	
	  pos = si*N+sj;
	  GSL_SET_COMPLEX(&square, 0, 0);

	  double data[] = { lat->cells[pos]->getTtautau(), -lat->cells[pos]->getTtaux(), -lat->cells[pos]->getTtauy(), -(it*dtau*a)*(it*dtau*a)*lat->cells[pos]->getTtaueta(),
	  		    lat->cells[pos]->getTtaux()  , -lat->cells[pos]->getTxx()  , -lat->cells[pos]->getTxy()  , -(it*dtau*a)*(it*dtau*a)*lat->cells[pos]->getTxeta(), 
	  		    lat->cells[pos]->getTtauy()  , -lat->cells[pos]->getTxy()  , -lat->cells[pos]->getTyy()  , -(it*dtau*a)*(it*dtau*a)*lat->cells[pos]->getTyeta(),
	  		    lat->cells[pos]->getTtaueta(), -lat->cells[pos]->getTxeta(), -lat->cells[pos]->getTyeta(), -(it*dtau*a)*(it*dtau*a)*lat->cells[pos]->getTetaeta()
	  };


	  
	  gsl_matrix_view m = gsl_matrix_view_array (data, 4, 4); //matrix
	  
	  gsl_vector_complex *eval = gsl_vector_complex_alloc (4); //eigenvalues are components of this vector
	  gsl_matrix_complex *evec = gsl_matrix_complex_alloc (4, 4); //eigenvectors are columns of this matrix
	  
	  gsl_eigen_nonsymmv_workspace * w = gsl_eigen_nonsymmv_alloc (4); //workspace
	  
	  gsl_eigen_nonsymmv (&m.matrix, eval, evec, w); // solve for eigenvalues and eigenvectors (without 'v' only compute eigenvalues)
	  
	  gsl_eigen_nonsymmv_free (w); // free memory associated with workspace
	  

	  //set to 'zero' 
	  lat->cells[pos]->setEpsilon( lat->cells[pos]->getTtautau() );
	  lat->cells[pos]->setutau(1.);
	  lat->cells[pos]->setux(0.);
	  lat->cells[pos]->setuy(0.);
	  lat->cells[pos]->setueta(0.);
	  
	  { // output:
	    int i;
	    foundU=0;
	    for (i = 0; i < 4; i++)
	      {
		gsl_complex eval_i = gsl_vector_complex_get (eval, i);
		gsl_vector_complex_view evec_i = gsl_matrix_complex_column (evec, i);
		
		GSL_SET_COMPLEX(&square, 0, 0);
		GSL_SET_COMPLEX(&tau2, a*it*dtau*a*it*dtau, 0);
		GSL_SET_COMPLEX(&euklidiansquare, 0, 0);
		
		for (int j = 0; j < 4; ++j)
		  {	
		    gsl_complex z = gsl_vector_complex_get(&evec_i.vector, j);
		    z_aux = gsl_complex_mul(tau2,z);	
		    euklidiansquare = gsl_complex_add(euklidiansquare, gsl_complex_mul(z,z));
		    
		    if (j==0)
		      square = gsl_complex_add(square, gsl_complex_mul(z,z));
		    else if (j<3)
		      square = gsl_complex_sub(square, gsl_complex_mul(z,z));
		    else
		      square = gsl_complex_sub(square, gsl_complex_mul(z_aux,z));
		  }
		
		GSL_SET_COMPLEX(&factor, sqrt(abs(GSL_REAL(euklidiansquare)/GSL_REAL(square))), 0);
		if(GSL_REAL(square)>0 )
		  {
		    eps = GSL_REAL(eval_i);
		    if(abs(GSL_IMAG(eval_i))>0.001 && si>0 && sj>0 && si<N-5 && sj<N-5)
		      {
			eps = lat->cells[pos]->getTtautau();
		      }
		  }
				
		GSL_SET_COMPLEX(&square, 0, 0);
	
		for (int j = 0; j < 4; ++j)
		  {	
		    gsl_complex z = gsl_vector_complex_get(&evec_i.vector, j);
		    z = gsl_complex_mul(z,factor);
		    z_aux = gsl_complex_mul(tau2,z);	
		   
		    if (j==0)
		      square = gsl_complex_add(square, gsl_complex_mul(z,z));
		    else if (j<3)
		      square = gsl_complex_sub(square, gsl_complex_mul(z,z));
		    else
		      square = gsl_complex_sub(square, gsl_complex_mul(z_aux,z));
		  }
		changeSign=0;
		// for the time-like eigenvector do the following (this is the flow velocity)
		if(GSL_REAL(square)>0)
		  {
		    foundU+=1;
		    for (int j = 0; j < 4; ++j)
		      {	
			gsl_complex z = gsl_vector_complex_get(&evec_i.vector, j);
			z = gsl_complex_mul(z,factor);
			
			if(j==0 && GSL_REAL(z)<0)
			  {
			    changeSign=1;
			    GSL_SET_COMPLEX(&z, -1.*GSL_REAL(z), -1.*GSL_IMAG(z));
			  }
			
			if(j>0 && changeSign==1)
			  {
			    GSL_SET_COMPLEX(&z, -1.*GSL_REAL(z), -1.*GSL_IMAG(z));
			  }
			
			if(j==0)
			  {
			    if(eps>0.1)
			      utau = GSL_REAL(z);
			    else 
			      utau=1.;
			  }
			if(j==1)
			  {
			    if(eps>0.1)
			      ux = GSL_REAL(z);
			    else 
			      ux=0.;
			  }
			if(j==2)
			  {
			    if(eps>0.1)
			      uy = GSL_REAL(z);
			    else 
			      uy=0.;
			  }
			if(j==3)
			  {
			    if(eps>0.1)
			      ueta = GSL_REAL(z);
			    else 
			      ueta=0.;
			  }
			if(abs(GSL_IMAG(z))>0.001 && eps>0.001 && si>10 && sj>10 && si<N-10  && sj<N-10)
			  {
			    utau=1.;
			    ux = 0.;
			    uy = 0.;
			    ueta=0.;
                            eps = lat->cells[pos]->getTtautau();
			  }
                      }
		    
		    lat->cells[pos]->setutau(utau);
		    lat->cells[pos]->setux(ux);
		    lat->cells[pos]->setuy(uy);
		    lat->cells[pos]->setueta(ueta);
		    lat->cells[pos]->setEpsilon(eps);

		    averageux+=ux*ux*eps;
		    averageuy+=uy*uy*eps;
		    averageueta+=ueta*ueta*eps*it*dtau*a*it*dtau*a;
		    averageeps+=eps;

		    count ++;
                  }
                
	      }
            
	  }

	    // write Tmunu in case no u was found
	    if ( foundU==0 ) 
	      {
                if(si==N/2 && sj==N/2)
                  {	cout << si << " " << sj << endl << endl;
                    cout << lat->cells[pos]->getTtautau() << " " << lat->cells[pos]->getTtaux() 
                         << " " << lat->cells[pos]->getTtauy() << " " << lat->cells[pos]->getTtaueta() << endl;
                    cout << lat->cells[pos]->getTtaux() << " " << lat->cells[pos]->getTxx() 
                         << " " << lat->cells[pos]->getTxy() << " " << lat->cells[pos]->getTxeta() << endl;
                    cout << lat->cells[pos]->getTtauy() << " " << lat->cells[pos]->getTxy() 
                         << " " << lat->cells[pos]->getTyy() << " " << lat->cells[pos]->getTyeta() << endl;
                    cout << lat->cells[pos]->getTtaueta() << " " << lat->cells[pos]->getTxeta() 
                         << " " << lat->cells[pos]->getTyeta() << " " << lat->cells[pos]->getTetaeta() << endl;
                    cout << "ux=" << ux << endl;
                    cout << "uy=" << uy << endl;
                    cout << "ueta=" << ueta << endl;
                  }
              }
	  
	    // compute pi^{\mu\nu}
	    if(utau==1 && ux ==0 && uy == 0 && ueta == 0)
	      {
		lat->cells[pos]->setpitautau(0.);
		lat->cells[pos]->setpixx(0.);
		lat->cells[pos]->setpiyy(0.);
		lat->cells[pos]->setpietaeta(0.);
		
		lat->cells[pos]->setpitaux(0.);
		lat->cells[pos]->setpitauy(0.);
		lat->cells[pos]->setpitaueta(0.);
		
		lat->cells[pos]->setpixeta(0.);
		lat->cells[pos]->setpixy(0.);
		lat->cells[pos]->setpiyeta(0.);
	      }
	    else
	      {
		lat->cells[pos]->setpitautau(lat->cells[pos]->getTtautau() - 4./3.*eps*utau*utau + eps/3.);
		lat->cells[pos]->setpixx(lat->cells[pos]->getTxx() - 4./3.*eps*ux*ux - eps/3.);
		lat->cells[pos]->setpiyy(lat->cells[pos]->getTyy() - 4./3.*eps*uy*uy - eps/3.);
		lat->cells[pos]->setpietaeta(lat->cells[pos]->getTetaeta() - 4./3.*eps*ueta*ueta - eps/3./it/dtau/a/it/dtau/a);
		
		lat->cells[pos]->setpitaux(lat->cells[pos]->getTtaux() - 4./3.*eps*utau*ux);
		lat->cells[pos]->setpitauy(lat->cells[pos]->getTtauy() - 4./3.*eps*utau*uy);
		lat->cells[pos]->setpitaueta(lat->cells[pos]->getTtaueta() - 4./3.*eps*utau*ueta);
		
		lat->cells[pos]->setpixeta(lat->cells[pos]->getTxeta() - 4./3.*eps*ux*ueta);
		lat->cells[pos]->setpixy(lat->cells[pos]->getTxy() - 4./3.*eps*ux*uy);
		lat->cells[pos]->setpiyeta(lat->cells[pos]->getTyeta() - 4./3.*eps*uy*ueta);
	      }
	    
	    gsl_vector_complex_free (eval);
	    gsl_matrix_complex_free (evec);
	
	}
    }

  cout << it*dtau*a << " average u^x=" << sqrt(averageux/averageeps) << endl;
  cout << it*dtau*a << " average u^y=" << sqrt(averageuy/averageeps) << endl;
  cout << it*dtau*a << " average tau u^eta=" << sqrt(averageueta/averageeps) << endl;

  double maxtime = param->getMaxtime(); // maxtime is in fm
  int itmax = static_cast<int>(floor(maxtime/(a*dtau)+1e-10));

  // output for hydro
  if(param->getWriteOutputs() > 0)
    {
      double alphas = param->getalphas();
      double g = param->getg();
      double gfactor;
      double Qs;
      int hx = param->getSizeOutput();
      int hy = hx;
      int heta = param->getEtaSizeOutput();
      double hL = param->getLOutput();
      double deta = param->getDetaOutput();
      double c = param->getc();
      double muZero = param->getMuZero();
      double g2mu2A, g2mu2B;
      double PI = param->getPi();

      if(hL>L)
	cout << "WARNING: hydro grid length larger than the computed one." << endl;
      
      int xpos, ypos, xposUp, yposUp, pos1, pos2, pos3;
      double fracx, fracy, x1, x2;
      double xlow, xhigh, ylow, yhigh;
      int pos4;
      double resultE, resultutau, resultux, resultuy, resultueta;
      double resultpi00, resultpi0x, resultpi0y, resultpi0eta;
      double resultpixy, resultpixeta, resultpiyeta, resultpixx, resultpiyy, resultpietaeta;

      double ha;
      ha = hL/static_cast<double>(hx);
      
      stringstream streuH_name;
      streuH_name << "epsilon-u-Hydro-t" << it*dtau*a << "-" << param->getMPIRank() << ".dat";
      string euH_name;
      euH_name = streuH_name.str();

      ofstream foutEps2(euH_name.c_str(),ios::out); 
      foutEps2 << "# dummy " << 1 << " etamax= " << heta
	       << " xmax= " << hx << " ymax= " << hy << " deta= " << deta 
	       << " dx= " << ha << " dy= " << ha << endl; 

			streuH_name.str("");
      streuH_name << "Tmunu-Hydro-t" << it*dtau*a << "-" << param->getMPIRank() << ".dat";
      euH_name = streuH_name.str();

      ofstream foutTmunu(euH_name.c_str(),ios::out); 
      foutTmunu << "# dummy " << 1 << " etamax= " << heta
	       << " xmax= " << hx << " ymax= " << hy << " deta= " << deta 
	       << " dx= " << ha << " dy= " << ha << endl; 
      
       for(int ieta=0; ieta<heta; ieta++) // loop over all positions
	{
	  for(int ix=0; ix<hx; ix++) // loop over all positions
	    {
	      for(int iy=0; iy<hy; iy++)
		{
		  x = -hL/2.+ha*ix;
		  y = -hL/2.+ha*iy;

			pos = ix*N+iy;
		  
		  //		  cout << ix << " " << iy << endl;
		  
		  if (abs(x) < L/2. && abs(y) < L/2.)
		    {
		      xpos = static_cast<int>(floor((x+L/2.)/a+0.0000000001));
		      ypos = static_cast<int>(floor((y+L/2.)/a+0.0000000001));
		      
		      
		      if(xpos<N-1)
			xposUp = xpos+1;
		      else
			xposUp = xpos;
		      
		      if(ypos<N-1)
			yposUp = ypos+1;
		      else
			yposUp = ypos;
		      
		      xlow = -L/2.+a*xpos;
		      ylow = -L/2.+a*ypos;
		      
		      xhigh = -L/2.+a*xposUp;
		      yhigh = -L/2.+a*yposUp;
		      
		      fracx = (x-xlow)/ha;
		      
		      pos1 = xpos*N+ypos;
		      pos2 = xposUp*N+ypos;
		      pos3 = xpos*N+yposUp;
		      pos4 = xposUp*N+yposUp;
		      

		      // -----------------------------epsilon--------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*abs(lat->cells[pos1]->getEpsilon())+fracx*abs(lat->cells[pos2]->getEpsilon());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*abs(lat->cells[pos3]->getEpsilon())+fracx*abs(lat->cells[pos4]->getEpsilon());
		      else
			x2 = 0.;
		      
		      fracy = (y-ylow)/ha;
		      
		      resultE = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------utau------------------------------------ //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getutau())+fracx*(lat->cells[pos2]->getutau());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getutau())+fracx*(lat->cells[pos4]->getutau());
		      else
			x2 = 0.;
		      
		      resultutau = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------ux-------------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getux())+fracx*(lat->cells[pos2]->getux());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getux())+fracx*(lat->cells[pos4]->getux());
		      else
			x2 = 0.;
		      
		      resultux = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------uy-------------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getuy())+fracx*(lat->cells[pos2]->getuy());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getuy())+fracx*(lat->cells[pos4]->getuy());
		      else
			x2 = 0.;
		      
		      resultuy = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------ueta------------------------------------ //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getueta())+fracx*(lat->cells[pos2]->getueta());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getueta())+fracx*(lat->cells[pos4]->getueta());
		      else
			x2 = 0.;
		      
		      resultueta = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pitautau-------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpitautau())+fracx*(lat->cells[pos2]->getpitautau());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpitautau())+fracx*(lat->cells[pos4]->getpitautau());
		      else
			x2 = 0.;
		      
		      resultpi00 = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pitaux---------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpitaux())+fracx*(lat->cells[pos2]->getpitaux());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpitaux())+fracx*(lat->cells[pos4]->getpitaux());
		      else
			x2 = 0.;
		      
		      resultpi0x = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pitauy---------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpitauy())+fracx*(lat->cells[pos2]->getpitauy());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpitauy())+fracx*(lat->cells[pos4]->getpitauy());
		      else
			x2 = 0.;
		      
		      resultpi0y = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pitaueta-------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpitaueta())+fracx*(lat->cells[pos2]->getpitaueta());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpitaueta())+fracx*(lat->cells[pos4]->getpitaueta());
		      else
			x2 = 0.;
		      
		      resultpi0eta = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pixy---------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpixy())+fracx*(lat->cells[pos2]->getpixy());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpixy())+fracx*(lat->cells[pos4]->getpixy());
		      else
			x2 = 0.;
		      
		      resultpixy = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pixeta---------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpixeta())+fracx*(lat->cells[pos2]->getpixeta());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpixeta())+fracx*(lat->cells[pos4]->getpixeta());
		      else
			x2 = 0.;
		      
		      resultpixeta = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------piyeta---------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpiyeta())+fracx*(lat->cells[pos2]->getpiyeta());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpiyeta())+fracx*(lat->cells[pos4]->getpiyeta());
		      else
			x2 = 0.;
		      
		      resultpiyeta = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pixx------------------------------------ //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpixx())+fracx*(lat->cells[pos2]->getpixx());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpixx())+fracx*(lat->cells[pos4]->getpixx());
		      else
			x2 = 0.;
		      
		      resultpixx = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------piyy------------------------------------ //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpiyy())+fracx*(lat->cells[pos2]->getpiyy());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpiyy())+fracx*(lat->cells[pos4]->getpiyy());
		      else
			x2 = 0.;
		      
		      resultpiyy = (1.-fracy)*x1+fracy*x2;
		      
		      // -----------------------------pietaeta-------------------------------- //
		      if(pos1>0 && pos1<(N)*(N) && pos2>0 && pos2<(N)*(N))
			x1 = (1-fracx)*(lat->cells[pos1]->getpietaeta())+fracx*(lat->cells[pos2]->getpietaeta());
		      else
			x1 = 0.;
		      
		      if(pos3>0 && pos3<N*N && pos4>0 && pos4<N*N)
			x2 = (1-fracx)*(lat->cells[pos3]->getpietaeta())+fracx*(lat->cells[pos4]->getpietaeta());
		      else
			x2 = 0.;
		      
		      resultpietaeta = (1.-fracy)*x1+fracy*x2;
		      
		      if(resultutau<1.)
			{
			  resultutau=1.;
			  resultux=0.;
			  resultuy=0;
			  resultueta=0;
			}
		      
		      
		      if(param->getRunningCoupling())
			{
				  // run with average Q_s only ! local makes no sense here (stuff has moved in the mean time)
			  alphas = 4.*PI/(9.* log(pow(pow(muZero/0.2,2./c) + pow(param->getRunWithThisFactorTimesQs()*param->getAverageQsmin()/0.2,2./c),c)));	  
			  gfactor = g*g/(4.*PI*alphas);
			}
		      else
			gfactor = 1.;
		      
		     	      
		      if(abs(0.1973269718*resultE*gfactor) > 0.0000000001)
			foutEps2 << -(heta-1)/2.*deta+deta*ieta << " " << x << " " << y << " " 
				 << abs(0.1973269718*resultE*gfactor) 
				 << " " << resultutau << " " << resultux << " " << resultuy << " " << resultueta 
				 << " " << resultpi00*gfactor << " " << resultpi0x*gfactor
				 << " " << resultpi0y*gfactor << " " << resultpi0eta*gfactor 
				 << " " << resultpixx*gfactor << " " << resultpixy*gfactor 
				 << " " << resultpixeta*gfactor << " " << resultpiyy*gfactor 
				 << " " << resultpiyeta*gfactor << " " << resultpietaeta*gfactor 
				 << endl; 
		      else
			foutEps2 << -(heta-1)/2.*deta+deta*ieta << " " << x << " " << y << " " 
				 << 0. << " " << 1. << " " << 0. << " " << 0. << " " << 0. 
				 << " " << 0. << " " << 0. << " " << 0. << " " << 0. << " " << 0. << " " << 0. 
				 << " " << 0. << " " << 0. << " " << 0. << " " << 0. << endl;
		    } 
		  else
		    foutEps2 << -(heta-1)/2.*deta+deta*ieta << " " << x << " " << y << " " 
			     << 0. << " " << 1. << " " << 0. << " " << 0. << " " << 0. 
			     << " " << 0. << " " << 0. << " " << 0. << " " << 0. << " " << 0. << " " << 0. 
			     << " " << 0. << " " << 0. << " " << 0. << " " << 0. << endl;

			foutTmunu << -(heta-1)/2.*deta+deta*ieta << " " << x << " " << y 
				<< " " << lat->cells[pos]->getTxx() 
				<< " " << lat->cells[pos]->getTyy() 
				<< " " << lat->cells[pos]->getTxy() 
				<< endl;

		}
	    }
	  foutEps2 << endl;
		foutTmunu << endl;
	}
       foutEps2.close();
       foutTmunu.close();
    }

  cout << "Wrote outputs" << endl;
  // done output for hydro
}


void MyEigen::test()
{
  gsl_complex square;
  gsl_complex factor;
  gsl_complex euklidiansquare;
  GSL_SET_COMPLEX(&square, 0, 0);

  double data[] = { 0.1,  -0.001, -0.01, 
		    0.001, -0.1,  -0.0001, 
		    0.01, -0.0001,  -0.1 };

  gsl_matrix_view m = gsl_matrix_view_array (data, 3, 3); //matrix

  gsl_vector_complex *eval = gsl_vector_complex_alloc (3); //eigenvalues are components of this vector
  gsl_matrix_complex *evec = gsl_matrix_complex_alloc (3, 3); //eigenvectors are columns of this matrix
  
  gsl_eigen_nonsymmv_workspace * w = gsl_eigen_nonsymmv_alloc (3); //workspace
  
  gsl_eigen_nonsymmv (&m.matrix, eval, evec, w); // solve for eigenvalues and eigenvectors (without 'v' only compute eigenvalues)
  
  gsl_eigen_nonsymmv_free (w); // free memory associated with workspace
  
  { // output:
    int i;
    
    for (i = 0; i < 3; i++)
      {
	gsl_complex eval_i = gsl_vector_complex_get (eval, i);
	gsl_vector_complex_view evec_i = gsl_matrix_complex_column (evec, i);

	printf ("before eigenvalue = %g + %gi\n", GSL_REAL(eval_i), GSL_IMAG(eval_i));
	GSL_SET_COMPLEX(&square, 0, 0);
	GSL_SET_COMPLEX(&euklidiansquare, 0, 0);

	for (int j = 0; j < 3; ++j)
	  {	
	    gsl_complex z = gsl_vector_complex_get(&evec_i.vector, j);
	    printf("%g + %gi\n", GSL_REAL(z), GSL_IMAG(z));
	    euklidiansquare = gsl_complex_add(euklidiansquare, gsl_complex_mul(z,z));
	    
	    if (j==0)
	      square = gsl_complex_add(square, gsl_complex_mul(z,z));
	    else
	      square = gsl_complex_sub(square, gsl_complex_mul(z,z));
	  }

	if(GSL_REAL(square)>0)
	  printf("before u_mu u^mu = %g + %gi\n", GSL_REAL(square), GSL_IMAG(square));
	else 
	  printf("before z_mu z^mu = %g + %gi\n", GSL_REAL(square), GSL_IMAG(square));

	GSL_SET_COMPLEX(&factor, sqrt(abs(GSL_REAL(euklidiansquare)/GSL_REAL(square))), 0);
	printf ("eigenvalue = %g + %gi\n", GSL_REAL(eval_i), GSL_IMAG(eval_i));
	
	GSL_SET_COMPLEX(&square, 0, 0);

	for (int j = 0; j < 3; ++j)
	  {	
	    gsl_complex z = gsl_vector_complex_get(&evec_i.vector, j);
	    z = gsl_complex_mul(z,factor);

	    printf("%g + %gi\n", GSL_REAL(z), GSL_IMAG(z));
    
	    if (j==0)
	      square = gsl_complex_add(square, gsl_complex_mul(z,z));
	    else
	      square = gsl_complex_sub(square, gsl_complex_mul(z,z));
	  }

	if(GSL_REAL(square)>0)
	  printf("u_mu u^mu = %g + %gi\n", GSL_REAL(square), GSL_IMAG(square));
	else 
	  printf("z_mu z^mu = %g + %gi\n", GSL_REAL(square), GSL_IMAG(square));
	  

      }
  }
  
  gsl_vector_complex_free (eval);
  gsl_matrix_complex_free (evec);
  
  exit(1);
}

