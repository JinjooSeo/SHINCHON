const int nbins = 100; //TODO: change me
//const int nbins = 150; //TODO: change me
const double max_x = 15; // TODO: change me
const bool saveQuarkDists = false; // TODO: whether to save wounded nucleon distributions in individual root files.

// Output file from runglauber should be called "outFile_${system}_${firstEvent}_${lastEvent}.root".
const string name_system = "AuAu"; //TODO: e.g. "He3Au" or "pPb". For accessing and saving the right files/directories.
const int energy = 200;
const int bmin = 0;
const int bmax = 18;
const int firstEvent = 0;
const int lastEvent = 1000;
const int scaler = 22;

// TODO: select an energy scaling value
//const double e0 = (scaler*0.1+0.1)*0.00150022 * TMath::Power(140.*max_x/(5.*nbins), 4); // 200 GeV (RHIC) 
//const double e0 = (scaler*0.1+0.3)*0.00150022 * TMath::Power(140.*max_x/(5.*nbins), 4); // >5 TeV (LHC) 
//const double e0 = (scaler*0.1+0.6)*0.00150022 * TMath::Power(140.*max_x/(5.*nbins), 4); // >5 TeV (LHC) //b 12-13 fm 
//const double e0 = (scaler*0.1+2.0)*0.00150022 * TMath::Power(140.*max_x/(5.*nbins), 4); // >5 TeV (LHC) //PbPb b 0-1 fm, scaler=3 
//const double e0 = (scaler*0.1+1.2)*0.00150022 * TMath::Power(140.*max_x/(5.*nbins), 4); // >5 TeV (LHC) //PbPb b 0-18 fm 
//const double e0 = (1.2-0.1*(scaler-9))*0.00150022 * TMath::Power(140.*max_x/(5.*nbins), 4); // >5 TeV (LHC) //PbPb b 0-18 fm 
const double e0 = TMath::Power(140.*max_x/(5.*nbins), 4); // >5 TeV (LHC) //PbPb b 0-18 fm 

void Hist2Txt_AuAu200GeV () {

	TString dir_name = Form("initedFiles_%s_%dGeV_b%d_%dfm_scaler%03d_bin%d",name_system.c_str(),energy,bmin,bmax,scaler,nbins);

	gSystem->Exec(Form("mkdir -p %s",dir_name.Data()));

	// inFile contains the histogram with the wounded nucleon distribution for all events.
	TFile* inFile = new TFile(Form("MCGlauber-%s-%dGeV-b%d-%dfm-bin100.root",name_system.c_str(),energy,bmin,bmax), "READ");
	cout << "OPEN: " << inFile->GetName() << endl;
	TFile* outRootFile = NULL;

	int npart;
	TTree *T = (TTree*)inFile->Get("lemon");
	T->SetBranchAddress("npart",&npart);

	TFile *inFile2 = new TFile("SONIC-ana-AuAu200GeV.root","read");
	TF1 *fSF = (TF1*)inFile2->Get("fSF");

  double max = 0; // for finding the maximum 
	for (int eventNum = firstEvent; eventNum < lastEvent; eventNum++) {
		TH2D* initedHist = (TH2D*)inFile->Get(Form("inited_event%i",eventNum));
		//initedHist->Scale (e0);

		T->GetEntry(eventNum);

		float ScaleFactor = fSF->Eval(npart);
		//float ScaleFactor = 1.4*fSF->Eval(npart);

		ofstream outFile;
		char outFileName[500];
		sprintf(outFileName, "./%s/event%d.dat",dir_name.Data(),eventNum);
		outFile.open(outFileName);

    for (int xbin = 1; xbin <= nbins; xbin++) {
      for (int ybin = 1; ybin <= nbins; ybin++) {
				double content = initedHist->GetBinContent(xbin, ybin); 

				if ( content<1e-5 ){
				//if ( 0 ){
					initedHist->SetBinContent(xbin, ybin, 1e-5*e0*ScaleFactor);
				}else{
					initedHist->SetBinContent(xbin, ybin, content*e0*ScaleFactor);
				}

				content = initedHist->GetBinContent(xbin, ybin); 

        outFile << content << "\t";
        if (content > max) max = content;
      }
      outFile << "\n";
    }
    outFile.close();

    if (saveQuarkDists) {
     // outRootFile copies the histogram with the distribution of wounded nucleons for a single event.
     // This can be used by diffusion.C in generating the initial positions of heavy qqbar pairs.
			TFile* outRootFile = new TFile(Form("./initedFiles_%s_%dGeV_b%d_%dfm_scaler%03d/event%d.root",name_system.c_str(),energy,bmin,bmax,scaler,eventNum), "RECREATE");
     outRootFile->cd();
     initedHist->Write();
    }
  }
  cout << "Max val: " << max << " GeV" << endl; // prints out the maximum initial energy density found

  inFile->Close();
  delete inFile;
}
