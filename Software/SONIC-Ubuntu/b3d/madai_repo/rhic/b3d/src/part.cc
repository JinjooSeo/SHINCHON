#ifndef __PART_CC__
#define __PART_CC__

#include "b3d.h"
using namespace std;
CB3D *CPart::b3d=NULL;

CPart::CPart(){
	p=new double[4];
	r=new double[4];
}
CPart::~CPart(){
	delete [] p;
	delete [] r;
}

void CPart::Copy(CPart *part){
	int alpha;
	tau0=part->tau0;
	tau_lastint=part->tau_lastint;
	tauexit=part->tauexit;
	y=part->y;
	eta=part->eta;
	for(alpha=0;alpha<4;alpha++){
		p[alpha]=part->p[alpha];
		r[alpha]=part->r[alpha];
	}
	mass=part->mass;
	resinfo=part->resinfo;
}

void CPart::InitH5(CPartH5 *partH5){
	double et;
	CResInfo *resinfoptr;
	int ID;
	//printf("IDset=%d, r=(%g,%g,%g,%g), p0=(%g,%g,%g,%g)\n",IDset,r[0],r[1],r[2],r[3],p0[0],p0[1],p0[2],p0[3]);
	b3d->reslist->GetResInfoptr(partH5->ID,resinfo);
	ID=resinfo->code;
	if(ID!=partH5->ID){
		printf("ID mismatch, ID=%d, partH5->ID=%d\n",ID,partH5->ID);
	}
	listid=partH5->listid;
	p[1]=partH5->px; p[2]=partH5->py; mass=partH5->mass; y=partH5->rapidity;
	r[1]=partH5->x; r[2]=partH5->y; tau0=partH5->tau; eta=partH5->eta;
	tau_lastint=tau0;
	b3d->reslist->GetResInfoptr(ID,*&resinfoptr);
	if(resinfoptr->decay==false){
		mass=resinfoptr->mass;
	}
	r[0]=tau0*cosh(eta);
	r[3]=tau0*sinh(eta);
	et=sqrt(p[1]*p[1]+p[2]*p[2]+mass*mass);
	p[3]=et*sinh(y);
	Setp0();
	if(tau0<0.0){
		printf("FATAL: tau0<0, tau0^2=%g\n",tau0);
		Print();
		exit(1);
	}
	if(b3d->BJORKEN && fabs(eta)>b3d->ETAMAX){
		CyclicReset();
		printf("performed cyclic reset in CPart::Init()\n");
	}
	SetInitialKey();

	active=false;
	ChangeMap(&(b3d->PartMap));
	b3d->AddAction_Activate(this);
	actionmother=b3d->nactions;
}

void CPart::Init(int IDset,double rxset,double ryset,double tauset,double etaset,double pxset,double pyset,double massset,double rapidityset){
	double et;
	CResInfo *resinfoptr;
	int ID;
	//cout << "Initializing particle" << endl;
	//printf("IDset=%d, r=(%g,%g,%g,%g), p0=(%g,%g,%g,%g)\n",IDset,r[0],r[1],r[2],r[3],p0[0],p0[1],p0[2],p0[3]);
	b3d->reslist->GetResInfoptr(IDset,resinfo);
	ID=resinfo->code;
	if(ID!=IDset){
		printf("ID mismatch, ID=%d, partH5->ID=%d\n",ID,IDset);
	}
	//listid=int(b3d->PartMap.size());
	p[1]=pxset; p[2]=pyset; mass=massset; y=rapidityset;
	r[1]=rxset; r[2]=ryset; tau0=tauset; eta=etaset;
	tau_lastint=tau0-1.0E-6;
	if(fabs(eta)>b3d->ETAMAX){
		printf("in part->init, eta out of bounds, =%g\n",eta);
	}
	b3d->reslist->GetResInfoptr(ID,*&resinfoptr);
	if(resinfoptr->decay==false){
		mass=resinfoptr->mass;
	}
	r[3]=tau0*sinh(eta);
	r[0]=tau0*cosh(eta);
	et=sqrt(p[1]*p[1]+p[2]*p[2]+mass*mass);
	p[3]=et*sinh(y);
	Setp0();
	if(tau0<0.0){
		printf("FATAL: tau0<0, tau0^2=%g\n",tau0);
		Print();
		exit(1);
	}
	if(b3d->BJORKEN && fabs(eta)>b3d->ETAMAX){
		CyclicReset();
		printf("performed cyclic reset in CPart::Init()\n");
	}
	SetInitialKey();
	active=false;
	if(b3d->COLLISIONS==true){
		ChangeMap(&(b3d->PartMap));
		b3d->AddAction_Activate(this);
	}
	else{
		ChangeMap(&(b3d->FinalPartMap));
		b3d->AddAction_Activate(this);
	}
	actionmother=b3d->nactions;
}

void CPart::CyclicReset(){
	double eta_offset,etamax=b3d->ETAMAX;
	double mt;
	while(fabs(eta)>etamax){
		eta_offset=2.0*etamax*floor(0.5+0.5*eta/etamax);
		eta-=eta_offset;
		y-=eta_offset;
		mt=sqrt(p[0]*p[0]-p[3]*p[3]);
		p[3]=mt*sinh(double(y));
		p[0]=sqrt(mt*mt+p[3]*p[3]);
		r[3]=tau0*sinh(double(eta));
		r[0]=tau0*cosh(double(eta));
	}
}

void CPart::Print(){
	printf("________________ PART INFO FOR listid=%d _____________________________\n",listid);
	printf("Minv^2=%g, ID=%d\n",p[0]*p[0]-p[1]*p[1]-p[2]*p[2]-p[3]*p[3],resinfo->code);
	printf("ID=%d, m=%g, roots=%g, tau0=%g=?%g, tauexit=%g\n r=(%g,%g,%g,%g) eta=%g=?%g, key=%d\n actionmother=%d, active=%d\n",
				 resinfo->code,resinfo->mass,sqrt(p[0]*p[0]-p[1]*p[1]-p[2]*p[2]-p[3]*p[3]),double(tau0),sqrt(r[0]*r[0]-r[3]*r[3]),tauexit,
				 r[0],r[1],r[2],r[3],eta,GetEta(b3d->tau),key,actionmother,int(active));
	printf("p=(%15.9e,%15.9e,%15.9e,%15.9e), y=%g\n",p[0],p[1],p[2],p[3],double(y));
	string currentmapname="IN CELL";
	if(currentmap==&(b3d->PartMap)) currentmapname="PartMap";
	if(currentmap==&(b3d->DeadPartMap)) currentmapname="DeadPartMap";
	if(currentmap==&(b3d->FinalPartMap)) currentmapname="FinalPartMap";
	printf("currentmap=%s\n",currentmapname.c_str());
	if(cell==NULL) printf("CELL=NULL\n");
	if(nextcell==NULL) printf("NEXTCELL=NULL\n");
	if(cell!=NULL) printf("Cell No: ix=%d, iy=%d, ieta=%d\n",cell->ix,cell->iy,cell->ieta);
	printf("________________________________________________________________________\n");
}

void CPart::ChangeMap(CPartMap *newmap){
	DeleteFromCurrentMap();
	AddToMap(newmap);
}

CPartMap::iterator CPart::DeleteFromCurrentMap(){
	CPartMap::iterator neighbor;
	CPartMap::iterator ppos=GetPos(currentmap);
	neighbor=ppos;
	neighbor++;
	if(ppos==currentmap->end()){
		printf("FATAL: In CPart::DeleteFromCurrentMap, can't find ppos!!!\n");
		Print();
		printf("currentmap has length %d\n",int(currentmap->size()));
		exit(1);
	}
	else currentmap->erase(ppos);
	currentmap=NULL;
	return neighbor;
}

CPartMap::iterator CPart::DeleteFromMap(CPartMap *partmap){
  CPartMap::iterator neighbor;
  CPartMap::iterator ppos=GetPos(partmap);
  neighbor=ppos;
  neighbor++;
  if(ppos==partmap->end()){
    printf("FATAL: In CPart::DeleteFromMap, can't find ppos!!!\n");
    Print();
    printf("partmap has length %d\n",int(partmap->size()));
    exit(1);
  }
  else{
    partmap->erase(ppos);
  }
  partmap=NULL;
  return neighbor;
}
/* old, causes segfaults?
CPartMap::iterator CPart::DeleteFromMap(CPartMap *partmap){
	CPartMap::iterator neighbor;
	CPartMap::iterator ppos=GetPos(partmap);
	neighbor=ppos;
	if(ppos==partmap->end()){
		printf("FATAL: In CPart::DeleteFromMap, can't find ppos!!!\n");
		Print();
		printf("partmap has length %d\n",int(partmap->size()));
		exit(1);
	}
	else{
		partmap->erase(ppos);
		neighbor++;
	}
	partmap=NULL;
	return neighbor;
	}*/

void CPart::AddToMap(CPartMap *newmap){
	newmap->insert(CPartPair(key,this));
	if(newmap==&b3d->PartMap || newmap==&b3d->DeadPartMap || newmap==&b3d->FinalPartMap)
		currentmap=newmap;
}

void CPart::AddToMap(CPartMap::iterator guess,CPartMap *newmap){
	newmap->insert(guess,CPartPair(key,this));
	currentmap=newmap;
}

void CPart::SubtractAction(CAction *action){
	CActionMap::iterator epos=action->GetPos(&actionmap);
	if(epos!=actionmap.end()) actionmap.erase(epos);
}

void CPart::AddAction(CAction *action){
	actionmap.insert(CActionPair(action->key,action));
}

void CPart::Propagate(double tau){
	double t0,vz,gamma,gammav;
	CPartMap *pmap=currentmap;
	CPartMap::iterator neighbor;
	if(active==true){
		eta=GetEta(tau);//y-asinh((tau0/tau)*sinh(y-eta));
		tau0=tau;
		gamma=cosh(eta);
		gammav=sinh(eta);
		t0=r[0];
		r[0]=tau0*gamma;
		r[3]=tau0*gammav;
		r[1]+=(p[1]/p[0])*(r[0]-t0);
		r[2]+=(p[2]/p[0])*(r[0]-t0);
	}
	else{
		r[0]=tau*cosh(eta);
		r[3]=tau*sinh(eta);
		tau0=tau;
	}
}

void CPart::SetInitialKey(){
	key=listid;
}

CPartMap::iterator CPart::GetPos(CPartMap *pmap){
	pair<CPartMap::iterator,CPartMap::iterator> ppospair;
	CPartMap::iterator ppos,pend;
	long long int count;
	count=pmap->count(key);
	ppospair=pmap->equal_range(key);
	ppos=ppospair.first;
	pend=ppospair.second;
	while(ppos->second!=this && ppos!=pend){
		++ppos;
	}
	if(ppos->second!=this){
		//printf("failed in CPart::GetPos()\n");
		//printf("key=%d, count=%d\n",key,count);
		return pmap->end();
	}
	else return ppos;
}

void CPart::CheckMap(CPartMap *expectedpartmap){
	if(currentmap!=expectedpartmap){
		printf("FATAL: XXXXXXXXX particle not in expected map XXXXXXXXX\n");
		if(currentmap==&(b3d->DeadPartMap)){
			printf("particle in DeadPartMap\n");
		}
		if(currentmap==&(b3d->PartMap)){
			printf("particlein PartMap\n");
		}
		Print();
		exit(1);
	}
}

double CPart::GetMass(){
	double s=p[0]*p[0]-p[1]*p[1]-p[2]*p[2]-p[3]*p[3];
	if(s<0.0){
		if(resinfo->code==22) s=0.0;
		else{
				printf("CPart::GetMass(), s<0??? =%g, code=%d, rapidity=%g\n",s,resinfo->code,y);
				Print();
				//exit(1);
		}
	}
	return sqrt(s);
}

void CPart::Setp0(){
	double m=resinfo->mass;
	p[0]=sqrt(p[1]*p[1]+p[2]*p[2]+p[3]*p[3]+m*m);
}

void CPart::SetY(){
	y=asinh(p[3]/GetMT());
}

double CPart::GetEta(double tau){
	double dy,deta,dtau0,dtau;
	if(active){
		dy=y;
		deta=eta;
		dtau0=tau0;
		dtau=tau;
		deta=dy-asinh((dtau0/dtau)*sinh(dy-deta));
		return deta;
	}
	else return eta;
}

double CPart::GetMT(){
	if(p[0]<fabs(p[3])){
		printf("CPart::GetMT, catastrophe\n");
		Print();
		exit(1);
	}
	return sqrt(p[0]*p[0]-p[3]*p[3]);
}

void CPart::KillActions(){
	CActionMap::iterator ep,epp;
	CAction *action;
	ep=actionmap.begin();
	while(ep!=actionmap.end()){
		action=ep->second;
		epp=ep; ++epp;
		action->Kill();
		ep=epp;
	}
	actionmap.clear();
}

void CPart::Kill(){
	KillActions();
	if(cell!=NULL){
		RemoveFromCell();
		cell=NULL;
	}
	DeleteFromCurrentMap();
	eta=0.0;
	tau0=tau_lastint=tauexit=-1.0;
	AddToMap(b3d->DeadPartMap.begin(),&(b3d->DeadPartMap));
	active=false;
}

void CPart::BjorkenTranslate(){
	double mt;
	if(cell->ieta==0){
		eta+=2.0*b3d->ETAMAX;
		y+=2.0*b3d->ETAMAX;
	}
	else{
		eta-=2.0*b3d->ETAMAX;
		y-=2.0*b3d->ETAMAX;
	}
	mt=sqrt(p[0]*p[0]-p[3]*p[3]);
	p[3]=mt*sinh(y);
	p[0]=mt*cosh(y);
	r[0]=tau0*cosh(eta);
	r[3]=tau0*sinh(eta);
}

void CPart::BjorkenUnTranslate(){
	double mt;
	if(cell->ieta==0){
		eta-=2.0*b3d->ETAMAX;
		y-=2.0*b3d->ETAMAX;
	}
	else{
		eta+=2.0*b3d->ETAMAX;
		y+=2.0*b3d->ETAMAX;
	}
	mt=sqrt(p[0]*p[0]-p[3]*p[3]);
	p[3]=mt*sinh(y);
	p[0]=mt*cosh(y);
	r[0]=tau0*cosh(eta);
	r[3]=tau0*sinh(eta);
}

void CPart::FindCollisions(){
	int ix,iy,ieta;
	double taucoll;
	CPart *part2,*part1=this;
	CPartMap::iterator ppos;
	CB3DCell *cell2;
	for(ix=0;ix<3;ix++){
		for(iy=0;iy<3;iy++){
			for(ieta=0;ieta<3;ieta++){
				cell2=cell->neighbor[ix][iy][ieta];
				if(cell2!=NULL){
					ppos=cell2->partmap.begin();
					while(ppos!=cell2->partmap.end()){
						part2=ppos->second;
						if(part1!=part2 && part1->actionmother!=part2->actionmother){
							b3d->FindCollision(part1,part2,taucoll);
						}
						++ppos;
					}
				}
			}
		}
	}
}

CB3DCell *CPart::FindCell(){
	int ieta,ix,iy;
	double deta=b3d->ETAMAX/double(b3d->NETA);
	ieta=lrint(floor((eta+b3d->ETAMAX)/deta));
	if(ieta<0 ||ieta>=2*b3d->NETA){
		return NULL;
	}
	double dx=b3d->XYMAX/double(b3d->NXY);
	double dy=dx;
	ix=lrint(floor((r[1]+b3d->XYMAX)/dx));
	if(ix<0 || ix>=2*b3d->NXY){
		return NULL;
	}
	iy=lrint(floor((r[2]+b3d->XYMAX)/dy));
	if(iy<0 || iy>=2*b3d->NXY){
		return NULL;
	}
	return b3d->cell[ix][iy][ieta];
}

void CPart::FindDecay(){
	CAction *action;
	if(b3d->DeadActionMap.size()==0){
		printf("MUST INCREASE NACTIONS_MAX\n");
		exit(1);
	}
	double t,gamma,vz,newt,newz;
	t=HBARC/resinfo->width;
	gamma=p[0]/GetMass();
	t=-t*gamma*log(b3d->randy->ran());
	vz=p[3]/p[0];
	newt=r[0]+t;
	newz=r[3]+vz*t;
	taudecay=sqrt(newt*newt-newz*newz);
	if(taudecay<tauexit || tauexit>b3d->TAUCOLLMAX || cell==NULL){
		b3d->AddAction_Decay(this,taudecay);
	}
}

void CPart::FindCellExit(){
	if(active){
		double t,taux,tauy,taueta,z;
		double etamax=cell->etamax,etamin=cell->etamin;
		nextcell=NULL;
		tauexit=1.0E50;
		taueta=taux=tauy=tauexit;
		double vx=p[1]/p[0];
		double vy=p[2]/p[0];
		double vz=p[3]/p[0];
		//Print();

		if(vx>0) t=(cell->xmax-r[1])/vx;
		else t=(cell->xmin-r[1])/vx;
		if(fabs(vx)<1.0E-20) t=1.0E50;
		t=t+r[0];
		z=r[3]+vz*(t-r[0]);
		taux=sqrt(t*t-z*z);

		if(vy>0) t=(cell->ymax-r[2])/vy;
		else t=(cell->ymin-r[2])/vy;
		if(fabs(vy)<1.0E-20) t=1.0E50;
		t=t+r[0];
		z=r[3]+vz*(t-r[0]);
		tauy=sqrt(t*t-z*z);
		//printf("t=%g, z=%g\n",t,z);

		if(y<cell->etamin){
			taueta=tau0*sinh(y-eta)/sinh(y-etamin);
		}
		else if(y>cell->etamax){
			taueta=tau0*sinh(y-eta)/sinh(y-etamax);
		}
		//printf("taux=%g, tauy=%g, taueta=%g\n",taux,tauy,taueta);

		//printf("b3d->tau=%g, tau0=%g, taux=%g, tauy=%g, taueta=%g\n",b3d->tau,tau0,taux,tauy,taueta);
		if(taueta<tau0){
			printf("In CPart::FindCellExit(),");
			printf("etamin=%g, etamax=%g\n",etamin,etamax);
			printf("eta=%g =? %g\n",eta,atanh(r[3]/r[0]));
			cout << "taueta= " << taueta << " tau0= " << tau0 << endl;
			cell->Print();
			Print();
			exit(1);
		}

		if(taux<tauy && taux<taueta){
			tauexit=taux;
			if(vx<0) nextcell=cell->neighbor[0][1][1];
			else nextcell=cell->neighbor[2][1][1];
		}
		else if (tauy<taueta){
			tauexit=tauy;
			if(vy<0) nextcell=cell->neighbor[1][0][1];
			else nextcell=cell->neighbor[1][2][1];
		}
		else{
			if(y<etamin || y>etamax){
				tauexit=taueta;
				if(y<etamin) nextcell=cell->neighbor[1][1][0];
				else nextcell=cell->neighbor[1][1][2];
			}
		}
		if(tauexit<b3d->TAUCOLLMAX)	b3d->AddAction_ExitCell(this);
		//if(listid==0)
			//printf("listid=0, tauexit=%g\n",tauexit);
	}
}

void CPart::FindActions(){
	if(active!=true){
		printf("CPart::FindActions(), trying to Reset Inactive particle\n");
		exit(1);
	}
	//KillActions();

	double oldp0=p[0];
	if(!resinfo->decay){
		p[0]=sqrt(p[1]*p[1]+p[2]*p[2]+p[3]*p[3]+resinfo->mass*resinfo->mass);
		mass=resinfo->mass;
	}
	else{
		mass=p[0]*p[0]-p[1]*p[1]-p[2]*p[2]-p[3]*p[3];
		if(mass<resinfo->minmass*resinfo->minmass){
			mass=sqrt(resinfo->minmass*resinfo->minmass+1.0);
			p[0]=sqrt(mass*mass+p[1]*p[1]+p[2]*p[2]+p[3]*p[3]);
		}
	}
	if(fabs(p[0]-oldp0)>0.01*p[0]){
		printf("Warning, CPart::FindActions(), oldp0=%g, p[0]=%g\n",oldp0,p[0]);
	}
	if(cell!=NULL){
		FindCellExit();
		FindCollisions();
	}
	if(resinfo->decay)
		FindDecay();
}

double CPart::GetPseudoRapidity(){
	double pmag,eta_ps;
	pmag=sqrt(p[1]*p[1]+p[2]*p[2]+p[3]*p[3]);
	eta_ps=atanh(p[3]/pmag);
	return eta_ps;
}

void CPart::Boost(double *u){
	BoostP(u);
	BoostR(u);
}

void CPart::BoostP(double *u){
	int alpha;
	double pprime[4];
	Misc::Boost(u,p,pprime);
	for(alpha=0;alpha<4;alpha++) p[alpha]=pprime[alpha];
	y=atanh(p[3]/p[0]);
}

void CPart::BoostR(double *u){
	int alpha;
	double rprime[4];
	Misc::Boost(u,r,rprime);
	for(alpha=0;alpha<4;alpha++) r[alpha]=rprime[alpha];
	eta=atanh(r[3]/r[0]);
	tau0=sqrt(r[0]*r[0]-r[3]*r[3]);
}

void CPart::Copy(CPartH5 *parth5){
	int alpha;
	double mt;
	tau0=parth5->tau;
	eta=parth5->eta;
	mass=parth5->mass;
	y=parth5->rapidity;
	r[0]=tau0*cosh(eta);
	r[3]=tau0*sinh(eta);
	r[1]=parth5->x;
	r[2]=parth5->y;
	p[1]=parth5->px;
	p[2]=parth5->py;
	mt=sqrt(mass*mass+p[1]*p[1]+p[2]*p[2]);
	p[0]=mt*cosh(y);
	p[3]=mt*sinh(y);
	b3d->reslist->GetResInfoptr(parth5->ID,resinfo);
	if(fabs(mass-resinfo->mass)>1.0E-3){
		printf("CPart::Copy, MASS OUT OF WHACK, =%g\n",mass);
		Print();
		exit(1);
	}
}

void CPart::RemoveFromCell(){
	if(cell!=NULL){
		CPartMap::iterator ppos=GetPos(&(cell->partmap));
		if(ppos==cell->partmap.end()){
			printf("FATAL: In CPart::RemoveFromCell, can't find ppos!!!\n");
			Print();
			printf("partmap has length %d\n",int(cell->partmap.size()));
			exit(1);
		}
		else{
			cell->partmap.erase(ppos);
		}
	}
}

void CPart::ChangeCell(CB3DCell *newcell){
	if(newcell!=cell){
		if(cell!=NULL)
			RemoveFromCell();
		if(newcell!=NULL){
			newcell->partmap.insert(CPartPair(key,this));
		}
		cell=newcell;
	}
}

#endif
