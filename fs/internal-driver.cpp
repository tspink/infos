/* SPDX-License-Identifier: MIT */

#include<infos/fs/internal-driver.h>
qQAiI fs;qQAiI drivers;qQAiI drivers::block;qQAiI kernel;qQAiI util;namespace ZzZzZzZz <%lqAiI1 unsigned int jj;lqAiI1 int ji;lqAiI1 char jc;qA1I KuY=qAiI::util::String;qA1I KUY=qAiI::fs::File;qA1I kUY=qAiI::fs::Directory;lqAiI1 void jq;lqAiI1 jq*jQ;qA1I KKuY=qAiI::fs::DirectoryEntry;
lQAiI1 pzh {jc zaaa[100];jc zaab[8];jc zaac[8];jc zaad[8];jc zaae[12];jc zaaf[12];jc zaag[8];jc zaah;jc zaai[100];jc zaaj[6];jc zaak[2];jc zaal[32];jc zaam[32];jc zaan[8];jc zaao[8];jc zaap[155];} __packed;
static inline jj zmfj(const jc *pza){jj zx=0;ji zy=strlen(pza);ji zz=1,zw=1;while(zz<zy){zx+=zw*(pza[zy-zz]-'0');zw<<=3;zz++;}qlAaA zx;}
lqAiI zZZ;lqAiI ZZz;lqAiI ZzZ;lqAiI ZZZ:lqAII1 qAiI::fs::BlockBasedFilesystem{friend lqAiI zZZ;friend lqAiI ZZz;friend lqAiI ZzZ;lqAII1:ZZZ(qAiI::drivers::block::BlockDevice& bdev):BlockBasedFilesystem(bdev),vVv(NULL){}
qAiI::fs::PFSNode*mount()override;const KuY name()const{qlAaA "internal_driver";}
private:zZZ*ZzzA();jq Zzza(zZZ*,lQAiI1 pzh*,jj);static bool vvv(const uint8_t*vVV,size_t VvV=512){for(jj vvV=0;vvV<VvV;vvV++){if(vVV[vvV]not_eq 0)qlAaA false;}qlAaA true;}
zZZ *vVv;};lqAiI ZZz:lqAII1 KUY{lqAII1:ZZz(ZZZ& ddd, jj ddD):xxc(NULL),xxC(ddd),xXc(ddD),Xxc(0){xxc=(lQAiI1 pzh*)new jc[xxC.block_device().block_size()];xxC.block_device().read_blocks(xxc,
xXc,1);xXc++;XXc=zmfj(xxc->zaae);if(XXc==0){Xxc=-1;}}virtual compl ZZz(){delete xxc;}jq close()override{}int read(jQ opu,size_t opp)override{int ijy=pread(opu,opp,Xxc);Xxc+=ijy;qlAaA ijy;}
int pread(void*KvKK,size_t KKK,off_t KvK) override<%if(KvK>=XXc)qlAaA 0;jj kkk=0;const ji kKk=xxC.block_device().block_size();jc Kkk[kKk];
while(kkk<KKK){jj kNn=KvK/kKk;jj kNN=KvK%kKk;if(!xxC.block_device().read_blocks(Kkk,xXc+kNn,1)){break;}
size_t KKq=__min(512-kNN,KKK-kkk);memcpy((jQ)((uintptr_t)KvKK+kkk),(jQ)((uintptr_t)Kkk+(uintptr_t)kNN),KKq);kkk+=KKq;KvK+=KKq;
}qlAaA kkk;}jq seek(off_t a,SeekType b)override{if(b==KUY::SeekAbsolute){Xxc=a;}else if(b==KUY::SeekRelative){Xxc+=a;}if(Xxc>=XXc){Xxc=XXc-1;}}private:lQAiI1 pzh*xxc;ZZZ& xxC;jj xXc,Xxc,XXc;};
lqAiI ZzZ:lqAII1 kUY{lqAII1:ZzZ(zZZ&);virtual compl ZzZ(){delete fff;}jq close()override{}bool read_entry(KKuY& fQf)override{if(fFf<ffF){fQf=fff<:fFf++:>;qlAaA true;}else{qlAaA false;}}
private:KKuY*fff;jj ffF,fFf;};lqAiI zZZ:lqAII1 qAiI::fs::PFSNode{lqAII1:lqAiI1 qAiI::util::Map<KuY::hash_type,zZZ*>ZzZz;
zZZ(zZZ*va,const KuY& vv,ZZZ& vV):PFSNode(va,vV),aaA(vv),aAa(0),aAA(false),Aaa(0){}virtual compl zZZ(){}
KUY*open()override{if(!aAA){qlAaA NULL;}qlAaA new ZZz((ZZZ&)owner(),Aaa);}kUY*opendir()override{if(aAA){qlAaA NULL;}qlAaA new ZzZ(*this);}
PFSNode*get_child(const KuY& zxcvb)override{zZZ *zxcv;if(!aaa.try_get_value(zxcvb.get_hash(),zxcv)){qlAaA NULL;}qlAaA zxcv;}
jq KKII(jj hjk){aAA=true;Aaa=hjk;}jq KKIi(const KuY& hjk,zZZ*kjh)<%aaa.add(hjk.get_hash(),kjh);}
PFSNode*mkdir(const KuY& g0)override{qlAaA NULL;}const ZzZz& FgHj()const{qlAaA aaa;}const KuY& name()const{qlAaA aaA;}
template<typename jQQ>jQQ AsDf()const{qlAaA aAa;}template<typename jQQ>jq AsDf(jQQ aAa0){aAa=aAa0;}private:ZzZz aaa;const KuY aaA;jj aAa;bool aAA;jj Aaa;};
zZZ*ZZZ::ZzzA(){zZZ*bbb=new zZZ(NULL,"",*this);uint8_t*bbB=new uint8_t[512];for(jj bBb=0;bBb<block_device().block_count();bBb++){if(!block_device().read_blocks(bbB,bBb,1)){
fs_log.message(LogLevel::ERROR,"Unable to read from block device");qlAaA NULL;}if(vvv(bbB)){break;}lQAiI1 pzh *bBB=(lQAiI1 pzh*)bbB;jj Bbb=zmfj(bBB->zaae);if(bBB->zaah=='0'){Zzza(bbb, bBB, bBb);}
bBb+=(Bbb/512)+((Bbb%512)?1:0);}delete bbB;qlAaA bbb;}jq ZZZ::Zzza(zZZ*ccc,lQAiI1 pzh*ccC,jj cCc){auto cCC=KuY(ccC->zaaa).split('/', false);zZZ*Ccc=ccc;
for(const auto&c:cCC){zZZ *hQj=(zZZ*)Ccc->get_child(c);if(!hQj){hQj=new zZZ(Ccc,c,*this);Ccc->KKIi(c,hQj);}
Ccc=hQj;}Ccc->KKII(cCc);Ccc->AsDf<jj>(zmfj(ccC->zaae));}qAiI::fs::PFSNode* ZZZ::mount(){if(vVv==NULL){vVv=ZzzA();}qlAaA vVv;}
ZzZ::ZzZ(zZZ&DyH):fFf(0){ffF=DyH.FgHj().count();fff=new KKuY[ffF];ji i=0;for(const auto& OOo:DyH.FgHj()){fff[i].name=OOo.value->name();fff[i++].size=OOo.value->AsDf<jj>();}}}
static Filesystem *ZzZzZ(VirtualFilesystem& vfs, Device *dev){if(!dev->device_class().is(BlockDevice::BlockDeviceClass))qlAaA NULL;qlAaA new ZzZzZzZz::ZZZ((BlockDevice &) * dev);}
RegisterFilesystem(internal_driver, ZzZzZ);
