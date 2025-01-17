#include "Style.h"

#include <TRandom.h>
#include <TRandom3.h>
#include <TH1.h>
#include <TFile.h>
#include <TProfile.h>
#include <TH2.h>
#include <TTree.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TProfile2D.h>

#include <iostream>
#include <fstream>

using namespace std;

Double_t fTsallis1S_v2(Double_t *x, Double_t *fpar);

void CalcRaaV3(){

	gStyle->SetOptStat(0);
	gStyle->SetPalette(55);

	gRandom = new TRandom3(0);

	const bool bDRAW = false; 
	const bool bSAVE = true; 
	const bool b2D = true;

	const int run_i = 0;
	const int run_f = 100;
	const int nrun = 1000;
	const float const_hbarc = 197.5; //MeV fm
	const float const_mY = 9.46; //GeV
	//const float const_mY = 4.18; //GeV
	const int nSAMP = 100; //times Ncoll

	ifstream fdata;

	char buf[500];
	vector<float> T[21];
	vector<float> Gdiss[21];

	float f_tmp[20];

	fdata.open("Gdiss0.dat");
	fdata.getline(buf,500);

	while ( fdata 
			>> f_tmp[0] >> f_tmp[1] >> f_tmp[2] >> f_tmp[3]
			>> f_tmp[4] >> f_tmp[5] >> f_tmp[6] >> f_tmp[7]
			>> f_tmp[8] >> f_tmp[9] >> f_tmp[10] >> f_tmp[11]
			){
		for (int ipt=0; ipt<11; ipt++){
			T[ipt].push_back(f_tmp[0]);
			Gdiss[ipt].push_back(f_tmp[ipt+1]);
		}
	}

	fdata.close();

	fdata.open("Gdiss1.dat");
	fdata.getline(buf,500);

	while ( fdata 
			>> f_tmp[0] >> f_tmp[1] >> f_tmp[2] >> f_tmp[3]
			>> f_tmp[4] >> f_tmp[5] >> f_tmp[6] >> f_tmp[7]
			>> f_tmp[8] >> f_tmp[9] >> f_tmp[10]
			){
		for (int ipt=0; ipt<10; ipt++){
			T[ipt+11].push_back(f_tmp[0]);
			Gdiss[ipt+11].push_back(f_tmp[ipt+1]);
		}
	}

	fdata.close();

	TGraphErrors *gGdiss[21];

	for (int ipt=0; ipt<21; ipt++){
		gGdiss[ipt] = new TGraphErrors;
		for (unsigned int iT=0; iT<T[ipt].size(); iT++){
			gGdiss[ipt]->SetPoint(iT, T[ipt][iT], Gdiss[ipt][iT]);
		}
	}

	TCanvas *c0;
	if ( bDRAW ){
		c0 = new TCanvas("c0","c0",1.2*500,500);
		SetPadStyle();

		htmp = (TH1D*)gPad->DrawFrame(170,0,500,200);
		SetHistoStyle("T [MeV]","#Gamma_{diss} [MeV]");

		for (int ipt=0; ipt<21; ipt++){
			gGdiss[ipt]->SetLineWidth(3);
			gGdiss[ipt]->SetLineColorAlpha(kGray,0.5);
			if ( ipt==0 ){
				gGdiss[ipt]->SetLineColor(2);
			}else if ( ipt==5 ){
				gGdiss[ipt]->SetLineColor(kGreen+2);
			}else if ( ipt==10 ){
				gGdiss[ipt]->SetLineColor(kBlue);
			}else if ( ipt==15 ){
				gGdiss[ipt]->SetLineColor(kMagenta);
			}else if ( ipt==20 ){
				gGdiss[ipt]->SetLineColor(1);
			}
			gGdiss[ipt]->Draw("C");
		}

		TLegend *leg = new TLegend(0.2,0.6,0.6,0.9);
		leg->SetFillStyle(0);
		leg->SetBorderSize(0);
		leg->SetTextFont(43);
		leg->SetTextSize(20);
		leg->AddEntry(gGdiss[0],"p_{T}=0 GeV/c","L");
		leg->AddEntry(gGdiss[5],"p_{T}=5 GeV/c","L");
		leg->AddEntry(gGdiss[10],"p_{T}=10 GeV/c","L");
		leg->AddEntry(gGdiss[15],"p_{T}=15 GeV/c","L");
		leg->AddEntry(gGdiss[20],"p_{T}=20 GeV/c","L");
		leg->Draw();
	}

	//return;

	//Upsilon beta vs. p 
	TF1 *fP = new TF1("fP","[0]*x/sqrt(1-x*x)",0,1);
	fP->SetParameter(0, const_mY);

	TCanvas *c1_;
	if ( bDRAW ){
		c1_ = new TCanvas("c1_","c1_",1.2*500,500);
		SetPadStyle();
		htmp = (TH1D*)gPad->DrawFrame(0,0,1,25);
		SetHistoStyle("#beta","p [GeV]");

		fP->SetLineColor(1);
		fP->SetLineWidth(3);
		fP->Draw("same");
	}

	//return;
	
	//Glauber
	//TFile *infileGlauber = new TFile("/alice/home/shlim/work/SHINCHON/SHINCHON/Software/SONIC-KIAF/input/MCGlauber-PbPb-5020GeV-b0-18fm.root","read");
	TFile *infileGlauber = new TFile("/alice/home/jseo/MCGlauber-PbPb-5020GeV-b0-18fm.root","read");
	TTree *TGlauber = (TTree*)infileGlauber->Get("lemon");
	int Gnpart, Gncoll;
	TGlauber->SetBranchAddress("npart",&Gnpart);
	TGlauber->SetBranchAddress("ncoll",&Gncoll);

	//QGP T profile
	TH2D *hTHydro[300];
	TGraphErrors *gTHydro[nrun];
	TF1 *fT2[nrun];
	double timeHydro[nrun][300] = {0.0}; 
	float errTHydro[nrun][300] = {0.0};
	float freezeT[nrun];
	float Npart[nrun];

	TProfile *hprofRAA_Npart = new TProfile("hprofRAA_Npart","",40,0,400);
	TProfile *hprofRAA_pT = new TProfile("hprofRAA_pT","",100,0,20);
	TProfile2D *hprofRAA_xy[nrun];


        TProfile *hprofRAA_Npart_rl = new TProfile("hprofRAA_Npart_rl","",40,0,400);
        TProfile *hprofRAA_pT_rl = new TProfile("hprofRAA_pT_rl","",100,0,20);
        TProfile2D *hprofRAA_xy_rl[nrun];

	TProfile *hprofRAA_Npart_allpT = new TProfile("hprofRAA_Npart_allpT","",40,0,400);

//****
        const int nPtBin = 10;
        double PtBin[nPtBin] = {
                0, 2, 4, 6, 8,
                12, 25, 50, 100, 1e4 };

        const int nNpartBin = 10;
        double NpartBin[nNpartBin] = {
                20, 50, 80, 100, 150,
                200, 250, 300, 350, 1e3 };
        const int nPhiBin = 33;
        double PhiBin[nPhiBin];
        for(int phi=0;phi<nPhiBin;phi++) PhiBin[phi] = (TMath::Pi()*2.0 / nPhiBin) * phi;

//      TProfile3D* hFinalState = new TProfile3D("hFinalState","hFinalState",
        TH3D* hFinalState = new TH3D("hFinalState","hFinalState",
                nPtBin-1, PtBin, nNpartBin-1, NpartBin, nPhiBin-1, PhiBin);
//**** temporary bin definition.

//Init
   	TF1 *fInitialUpsilon = new TF1("fInitialUpsilon",fTsallis1S_v2,0,30,3);
        fInitialUpsilon -> SetParameters(  1.06450e+00 ,  7.97649e-01 , 100);
//Init

	for (int irun=run_i; irun<run_f; irun++){

		cout << "Scan event #" << irun << endl;

		if ( b2D ){
			hprofRAA_xy[irun] = new TProfile2D(Form("hprofRAA_xy_run%05d",irun),"",100,-15,15,100,-15,15);
			hprofRAA_xy_rl[irun] = new TProfile2D(Form("hprofRAA_xy_rl_run%05d",irun),"",100,-15,15,100,-15,15);
		}

		TFile *infileHydro = new TFile(Form("/alice/data/shlim/SONIC_profile_PbPb5TeV_0_18fm/SONIC_profile_PbPb5TeV_0_18fm_event%05d.root",irun),"read");
		//TFile *infileHydro = new TFile(Form("SONIC_profile_PbPb5TeV_0_18fm_event%05d.root",irun),"read");
		TH1D *htimeHydro = (TH1D*)infileHydro->Get("Time");
		int ntimeHydro = (int)htimeHydro->GetEntries();

		//Glauber info
		TGlauber->GetEntry(irun);
		Npart[irun] = Gnpart;

		TH2D *hGlauber = (TH2D*)infileGlauber->Get(Form("inited_event%d",irun));

		//Load histograms
		for (int it=0; it<ntimeHydro; it++){
			hTHydro[it] = (TH2D*)infileHydro->Get(Form("T_%d",it));
			timeHydro[irun][it] = htimeHydro->GetBinContent(it+1);
		}

		const int nY = nSAMP * Gncoll;

		//Upsilon
		for (int iY=0; iY<nY; iY++){

			//Momentum
			//double pT = 20.0*gRandom->Rndm();
			//double pT = 3.0;
			double pT = fInitialUpsilon->GetRandom();
			double phi = (gRandom->Rndm()-0.5)*TMath::TwoPi(); 
			double px = pT*cos(phi);
			double py = pT*sin(phi);

			int pTbin = int(pT);
			if ( pTbin>=20 ) continue;

			//Beta
			double bx = fP->GetX(fabs(px)); 
			double by = fP->GetX(fabs(py)); 

			if ( px<0 ) bx *= -1;
			if ( py<0 ) by *= -1;

			//Position
			double vx, vy;
			hGlauber->GetRandom2(vx, vy);

			double vx0 = vx;
			double vy0 = vy;

			double modF = 1.0;
			int nFO = 0;

			double det_flg = 1.0;

			//Pre-Hydro 
			{
				float TPre = 1.20*hTHydro[0]->GetBinContent(hTHydro[0]->FindBin(vx, vy))*1000.;
				if ( TPre>170.0 ){
					float GdissPre0 = gGdiss[pTbin]->Eval(TPre);
					float GdissPre1 = gGdiss[pTbin+1]->Eval(TPre);
					float GdissPre = GdissPre0 + (GdissPre1 - GdissPre0)*(pT - pTbin);
					modF = exp(-(0.2)*GdissPre/const_hbarc);
					if( exp(-(0.2)*GdissPre/const_hbarc) < gRandom->Rndm() ) det_flg = 0.0;
				}else{
					modF = 1.0;
					nFO++;
				}
			}

			//Time evolution
			for (int it=0; it<ntimeHydro-1; it++){

				if ( nFO>=10 ) break;

				float dt = timeHydro[irun][it+1] - timeHydro[irun][it];
				float dx = bx*dt;
				float dy = by*dt;

				float THydro0 = hTHydro[it]->GetBinContent(hTHydro[it]->FindBin(vx, vy))*1000.;
				float THydro1 = hTHydro[it+1]->GetBinContent(hTHydro[it+1]->FindBin(vx+dx, vy+dy))*1000.;

				if ( (THydro0+THydro1)/2<170.0 ){
					vx += dx;
					vy += dy;
					nFO++;
					continue;
				}

				float Gdiss0 = gGdiss[pTbin]->Eval((THydro0+THydro1)/2);
				float Gdiss1 = gGdiss[pTbin+1]->Eval((THydro0+THydro1)/2);
				float Gdiss = Gdiss0 + (Gdiss1 - Gdiss0)*(pT - pTbin); 
				modF *= exp(-(dt)*Gdiss/const_hbarc);
				if( exp(-(dt)*Gdiss/const_hbarc) < gRandom->Rndm() ) det_flg = 0.0;

				vx += dx;
				vy += dy;

			}//it

			if ( pT>2.0 && pT<4.0 ){
				hprofRAA_Npart->Fill(Npart[irun], modF);
				hprofRAA_Npart_rl->Fill(Npart[irun], det_flg );
				if ( b2D ){
					hprofRAA_xy[irun]->Fill(vx0, vy0, modF);
					hprofRAA_xy_rl[irun]->Fill(vx0, vy0, det_flg );
				}
			}
			hFinalState->Fill( pT, Npart[irun], TVector2(vx,vy).Phi(), det_flg );
			
			hprofRAA_Npart_allpT->Fill(Npart[irun], modF);

			hprofRAA_pT->Fill(pT, modF);
			hprofRAA_pT_rl->Fill(pT, det_flg );
		}//iY

		infileHydro->Close();
		delete infileHydro;

	}

	if ( bDRAW ){

		TCanvas *c1 = new TCanvas("c1","c1",1.2*2*500,500);
		c1->Divide(2,1);

		c1->cd(1);
		SetPadStyle();
		gPad->SetRightMargin(0.13);
		htmp = (TH1D*)gPad->DrawFrame(-10,-10,10,10);
		htmp->GetXaxis()->SetTitle("x [fm]");
		htmp->GetXaxis()->SetLabelSize(0.04);
		htmp->GetXaxis()->SetTitleSize(0.05);
		htmp->GetYaxis()->SetTitle("y [fm]");
		htmp->GetYaxis()->SetLabelSize(0.04);
		htmp->GetYaxis()->SetTitleSize(0.05);

		TH2D *hGlauber = (TH2D*)infileGlauber->Get("inited_event0");
		hGlauber->GetZaxis()->SetTitle("");
		hGlauber->Draw("colz same");

		c1->cd(2);
		SetPadStyle();
		gPad->SetRightMargin(0.13);
		htmp = (TH1D*)gPad->DrawFrame(-10,-10,10,10);
		htmp->GetXaxis()->SetTitle("x [fm]");
		htmp->GetXaxis()->SetLabelSize(0.04);
		htmp->GetXaxis()->SetTitleSize(0.05);
		htmp->GetYaxis()->SetTitle("y [fm]");
		htmp->GetYaxis()->SetLabelSize(0.04);
		htmp->GetYaxis()->SetTitleSize(0.05);

		//hprofRAA_xy[0]->SetMaximum(1);
		//hprofRAA_xy[0]->SetMinimum(0);
		//hprofRAA_xy[0]->Draw("colz same");
	}

	if ( bDRAW ){

		TCanvas *c2 = new TCanvas("c2","c2",1.2*2*500,500);
		c2->Divide(2,1);

		c2->cd(1);
		SetPadStyle();
		gPad->SetRightMargin(0.13);
		htmp = (TH1D*)gPad->DrawFrame(0,0,400,1.2);
		htmp->GetXaxis()->SetTitle("N_{part}");
		htmp->GetXaxis()->SetLabelSize(0.04);
		htmp->GetXaxis()->SetTitleSize(0.05);
		htmp->GetYaxis()->SetTitle("R_{AA} at p_{T}=3 GeV/c");
		htmp->GetYaxis()->SetLabelSize(0.04);
		htmp->GetYaxis()->SetTitleSize(0.05);

		hprofRAA_Npart->SetLineWidth(2);
		hprofRAA_Npart->Draw("same");


		c2->cd(2);
		SetPadStyle();
		gPad->SetRightMargin(0.13);
		htmp = (TH1D*)gPad->DrawFrame(0,0,20,1.2);
		htmp->GetXaxis()->SetTitle("p_{T} (GeV/c)");
		htmp->GetXaxis()->SetLabelSize(0.04);
		htmp->GetXaxis()->SetTitleSize(0.05);
		htmp->GetYaxis()->SetTitle("R_{AA}");
		htmp->GetYaxis()->SetLabelSize(0.04);
		htmp->GetYaxis()->SetTitleSize(0.05);

		hprofRAA_pT->SetLineWidth(2);
		hprofRAA_pT->Draw("same");

	}

	if ( bSAVE ){

		TFile *outfile = new TFile(Form("outfile_RaaV2_%04d_%04d.root",run_i,run_f),"recreate");

		hprofRAA_Npart->Write();
		hprofRAA_pT->Write();
		hprofRAA_Npart_allpT->Write();

		if ( b2D ){
			for (int irun=run_i; irun<run_f; irun++){
				hprofRAA_xy[irun]->Write();
			}
		}
		hFinalState->Write();
	}


	//hprofRAA_xy[0]->Draw("colz");

	return;

	//Calculate RAA



	return;




}

Double_t fTsallis1S_v2(Double_t *x, Double_t *fpar)
{
  Float_t xx = x[0];
  Double_t Y1Smass = 9.46;
  Double_t q = fpar[0];
  Double_t T = fpar[1];
  Double_t c = fpar[2];
  Double_t mT = TMath::Sqrt(Y1Smass*Y1Smass+xx*xx);
  Double_t pow = TMath::Power((1+(q-1)*mT/T),(-q/(q-1)));

  Double_t f = c*mT*xx*pow;
  return f;
}
