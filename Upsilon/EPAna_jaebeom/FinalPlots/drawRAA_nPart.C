#include <iostream>
#include "../Style_jaebeom_woFrameLegend.h"
#include "../SHINCHONLegend_raaCent.C"
#include "../tdrstyle.C"

using namespace std;

void drawRAA_nPart(int nrun = 10, int kInitPos=1, int kDatSel=0, bool isLine = true)
{
  setTDRStyle();
  writeExtraText=false;
  int iPeriod = -1;
  int iPos = 33;
  bool drawInner = true;

  double xmax = 420.0;
  double xmin = 0.0;
  double xmin_int = 0.5;
  double xmax_int = 1.5;
  double ymin =0;
  double ymax = 1.3;
  double boxw = 6.5; // for syst. box (vs cent)
  double boxw_int = 0.09;
  double xlonger = 120;

  const char* fPosStr;
  if(kInitPos==0) fPosStr = "InitPosZero";
  else if(kInitPos==1) fPosStr = "InitPosGlauber";
  else if(kInitPos==2) fPosStr = "InitPosMean";
  
  const int nData = 3;
  enum fData {CMS_502, CMS_276, STAR_200,};
  const char* fDataStr[nData] = {"CMS502","CMS276","STAR200"};

  ////////////////////////////////////////////////////////////////
  //// read input file : value & stat.
  TFile *rf = new TFile(Form("../res/SavedPlots_%s_%s_nRun%d.root",fDataStr[kDatSel],fPosStr,nrun),"read");
  TH1D* hRAA_npart = (TH1D*) rf->Get("hNpart_allEvt_RAA");
  TH1D* hRAA_int = (TH1D*) rf->Get("hRAA_int");
  TGraphErrors* gRAA_npart = new TGraphErrors();
  TGraphErrors* gRAA_npart_err = new TGraphErrors();
  int ipgr=0;
  for(int ip=0; ip<hRAA_npart->GetNbinsX();ip++){
    if(hRAA_npart->GetBinContent(ip+1)==0) continue;
    gRAA_npart->SetPoint(ipgr,hRAA_npart->GetBinCenter(ip+1),hRAA_npart->GetBinContent(ip+1));
    gRAA_npart->SetPointError(ipgr,hRAA_npart->GetBinWidth(ip+1)/2,hRAA_npart->GetBinError(ip+1));
    if(isLine){
      gRAA_npart_err->SetPoint(ipgr,hRAA_npart->GetBinCenter(ip+1),hRAA_npart->GetBinContent(ip+1));
      gRAA_npart_err->SetPointError(ipgr,hRAA_npart->GetBinWidth(ip+1)/2,hRAA_npart->GetBinError(ip+1));
      gRAA_npart->SetPointError(ipgr,0,0);
    }
    ipgr++;
  }
  TGraphErrors* gRAA_int = new TGraphErrors();
  gRAA_int->SetPoint(0,1,hRAA_int->GetBinContent(1));
  gRAA_int->SetPointError(0,boxw_int,hRAA_int->GetBinError(1));

  SetGraphStyle(gRAA_npart,1,0);
  SetGraphStyle(gRAA_int,1,0);
  
  if(isLine){ 
    SetGraphStyleSys(gRAA_npart,1);
    gRAA_npart->SetLineWidth(3);
    gRAA_npart->SetMarkerSize(0);
    gRAA_int->SetLineWidth(3);
    gRAA_int->SetMarkerSize(0);
    gRAA_npart_err->SetMarkerSize(0);
    gRAA_npart_err->SetFillColorAlpha(kBlue-3,0.4);
  }

  SetGraphAxis(gRAA_npart,"N_{part}","R_{AA}");

  
  //// draw  

  TCanvas* c1 = new TCanvas("c1","c1",600+xlonger,600);

  c1->cd();
  gRAA_npart->GetYaxis()->SetTitleOffset(1.4);
  gRAA_npart->GetXaxis()->SetTitleOffset(1.1);
  gRAA_npart->GetXaxis()->SetTitleSize(0.055);
  gRAA_npart->GetXaxis()->SetLabelSize(0.043);
  gRAA_npart->GetYaxis()->SetLabelSize(0.043);
  gRAA_npart->GetYaxis()->SetNdivisions(510);
  gRAA_npart->GetXaxis()->SetLimits(xmin,xmax);
  gRAA_npart->GetXaxis()->SetRangeUser(xmin,xmax);
  gRAA_npart->SetMinimum(ymin);
  gRAA_npart->SetMaximum(ymax);

  TPad* pad_diff = new TPad("pad_diff", "",0, 0, 600/(600.+xlonger), 1.0); // vs centrality
  pad_diff->SetLeftMargin(0.2);
  pad_diff->SetRightMargin(0);
  pad_diff->SetBottomMargin(0.14);
  pad_diff->SetTopMargin(0.067);
  TPad* pad_int = new TPad("pad_int", "",600/(600.+xlonger), 0, 1.0, 1.0); // centrality-integrated
  pad_int->SetBottomMargin(0.14);
  pad_int->SetTopMargin(0.067);
  pad_int->SetLeftMargin(0);
  pad_int->SetRightMargin(0.032*600/xlonger);

  //// --- 1st pad!!!   
  c1->cd();
  pad_diff->Draw(); 
  pad_diff->cd(); 
  if(!isLine) gRAA_npart->Draw("AP");
  else if(isLine){
    gRAA_npart->Draw("AL");
    gRAA_npart_err->Draw("LE3 same");
  }
  dashedLine(xmin,1.,xmax,1.,1,1);
  
  //// legend
  double legposx1 = 0.59;
  double legposx2 = 0.78;
  double legposy1 = 0.62;
  double legposy2 = 0.7;
  double labtextsize=0.0385;

  TLegend *leg= new TLegend(legposx1,legposy1,legposx2,legposy2);
  SetLegendStyle(leg);
  if(!isLine) leg->AddEntry(gRAA_npart,"#varUpsilon(1S) (#eta#approx0)","pe");
  else if(isLine) leg->AddEntry(gRAA_npart,"#varUpsilon(1S) (#eta#approx0)","l");
  leg->SetTextSize(labtextsize);
  leg->Draw("same");
  
  double lab_posx = 0.275; double lab_posy = 0.87; double lab_pos_diff = 0.269;
  drawGlobText("PbPb, #sqrt{s_{NN}} = 5.02 TeV", lab_posx, lab_posy, 1, labtextsize);

  pad_diff->Update();
  SHINCHONLegend_raaCent(pad_diff, iPeriod, iPos);
  pad_diff->Update();
  //// --- 2nd pad!!!   
  c1->cd();
  pad_int->Draw(); 
  pad_int->cd(); 
  
  //// for int
  gRAA_int->GetXaxis()->SetLimits(xmin_int,xmax_int);
  gRAA_int->SetMinimum(ymin);
  gRAA_int->SetMaximum(ymax);
  gRAA_int->GetXaxis()->SetNdivisions(101);
  gRAA_int->GetXaxis()->SetLabelSize(0);
  gRAA_int->GetYaxis()->SetTickLength(0.03*600/xlonger);
  gRAA_int->GetYaxis()->SetLabelSize(0);
 
  if(!isLine) gRAA_int->Draw("AP"); 
  else if(isLine){
    gRAA_int->SetFillColorAlpha(kBlue-3,0.4);
    gRAA_int->Draw("ALE");
  }
  dashedLine(0.,1.,xmax_int,1.,1,1);
  
  pad_int->Update();
  //// draw text
  double sz_allign = 0.034;
  double sz_init = 0.574; double sz_step = 0.0558;
  TLatex* globtex = new TLatex();
  globtex->SetNDC();
  globtex->SetTextFont(42);
  globtex->SetTextSize(0.0384);
  globtex->SetTextAlign(22); //center-center
  globtex->SetTextSize(0.038*600./xlonger);
  globtex->DrawLatex(0.5*(1-0.032*600/xlonger), sz_init-sz_step-sz_allign, "<Npart>");

	c1->Update();
  
  string savedir = Form("plots_%s",fPosStr);
  void * dirf = gSystem->OpenDirectory(savedir.c_str());
  if(dirf) gSystem->FreeDirectory(dirf);
  else {gSystem->mkdir(savedir.c_str(), kTRUE);}

  c1->SaveAs(Form("%s/RAA_vs_npart_%s_isLine%d.pdf",savedir.c_str(),fDataStr[kDatSel],isLine));
	return;

} // end of main func.

