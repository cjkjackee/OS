#include <iostream>
#include <stdio.h>
#include <vector>
#include <cstdint>
#include <inttypes.h>
using namespace std;

#define max 65535
#define pagesize 256
#define framesize 256

int main(int argc,char** argv){

	vector<int> address;
	vector<int>tlb;
	vector<int>framenum;
	tlb.resize(16,-1);

	int sizeof_testcase;
	int temp;
	int8_t value;
	int address_add;
	int miss_count=0;
	int hit_count=0;
	bool frame_found=0;
	bool tlb_hit=0;
	int k;
	int current_frame=-1;
	if (argc!=3){
	fprintf(stderr,"usage:backing_store.bin address.txt\n");
	}

	FILE *fp;
	FILE *addr;
	FILE *ans;

	fp=fopen(argv[1],"r");
	addr=fopen(argv[2],"r");
	ans=fopen("results.txt","w");
	fscanf(addr,"%d",&sizeof_testcase);
	//cout<<"number of test case:"<<sizeof_testcase<<"\n";
	for(int i=0;i<sizeof_testcase;i++){
		fscanf(addr,"%d",&temp);
		address.push_back(temp);
		fseek(fp,temp,SEEK_SET);
		fread(&value,1,1,fp);
	
		
//framecount start

		for(k=0;k<framenum.size();k++){			
			if(framenum[k]==temp/pagesize){
			frame_found=1;
				break;
			}
		}
		if(frame_found==0){
			current_frame++;
			address_add=current_frame*pagesize;
			framenum.push_back(temp/pagesize);
		}
		else{
			address_add=k*pagesize;
			frame_found=0;
		}

//framecount end
// tlbstart
		for(k=0;k<tlb.size();k++){
			if(tlb[k]==temp/pagesize){
				tlb_hit=1;
				break;
			}
		}
		if(tlb_hit==0){
			miss_count++;
			tlb.erase(tlb.begin());
			tlb.push_back(temp/pagesize);
		}
		else{
			tlb.erase(tlb.begin()+k);
			tlb.push_back(temp/pagesize);
			tlb_hit=0;
			hit_count++;
		}





// tlb end		



		fprintf(ans,"%d ",(address[i]%pagesize)+address_add);
		fprintf(ans,"%" PRId8 "\n",value);
	}

	
	fprintf(ans,"TLB hits: %d\n",hit_count);
	fprintf(ans,"Page Faults: %d\n",miss_count);
	fclose(fp);
	fclose(addr);
	fclose(ans);
	return 0;
}