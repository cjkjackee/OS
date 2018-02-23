#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
using namespace std;

struct data
{
	int address;
	int hit;
};

vector<int> record;
map<int,struct data> tlb;
vector<int> page_table;
vector<vector<char> > memory;

int main (int argc, char** argv)
{
	FILE* store;	
	fstream fin, fout;	
	int address, n;
	int offset, pageno;
	int phy_address;
	bool retry;
	unsigned int tlb_hit;
	unsigned int page_fault;

	if (argc != 3)
	{
		cout << "Usage: ./a.out BACKING_STORE.bin address.txt" << endl;
		exit(1);
	}	

	page_table.assign(256,-1);
	memory.clear();
	store = fopen(argv[1],"r");
	tlb_hit = 0;
	page_fault = 0;

	fin.open(argv[2]);
	fout.open("results.txt",fstream::out);
	fin >> n;
	for (int x=0;x<n;++x)
	{
		fin >> address;
		offset = address & 0xff;
		pageno = address >> 8;

		if(tlb.find(pageno)!=tlb.end())
		{
			tlb[pageno].hit++;
			++tlb_hit;
		}
		else
		{
			if (page_table[pageno]!=-1)
			{
				if(tlb.size()>15)
				{
					int tmp = -1;
					for (vector<int>::iterator it=record.begin();it!=record.end();++it)
					{
						int hit_time = x;
						if (tlb[*it].hit < hit_time)
							hit_time = tlb[*it].hit;
						if (tlb[*it].hit == 1)
							break;
					}
					map<int,data>::iterator it2 = tlb.find(tmp);
					tlb.erase(it2);
					for (vector<int>::iterator it=record.begin();it!=record.end();++it)
					{
						if (*it == tmp)
						{
							record.erase(it);
							break;
						}
					}
				}
				tlb[pageno].address = page_table[pageno];
				tlb[pageno].hit = 1;
				record.push_back(pageno);
			}
			else
			{
				++page_fault;
				int tmp = pageno;
				char value[256];	
				vector<char> s;

				page_table[pageno] = memory.size();
				tlb[pageno].address = page_table[pageno];
				fseek(store, tmp*256, SEEK_SET);
				fread(&value, 1, 256, store);
				for (int i=0;i<256;++i)
					s.push_back(value[i]);
				memory.push_back(s);
			}
		}
		phy_address = tlb[pageno].address;

		char value = memory[phy_address][offset];
		char str[10];
		phy_address = phy_address << 8;
		phy_address ^= offset;

		fout << phy_address;
		snprintf(str,sizeof(str)," %" PRId8 "\n",value);
		fout << str;
	}
	fout << "TLB hits: " << tlb_hit << endl;
	fout << "Page Faults: " << page_fault << endl;

    return 0;
}
