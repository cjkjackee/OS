#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
using namespace std;

class process
{
public:
    process(){};
    ~process(){};
    int arrival;
    int burst;
    int now;
    double wait;
    double turn;
};

int main ()
{
    fstream fin;
    int n;
    int t;
    int quantum1;
    int quantum2;
    double ave_turn=0;
    double ave_wait=0;
    process proc;
    vector<process> p;
    deque<process*> coming,Hpriority, Mpriority, Lpriority;

    fin.open("Q4.txt");
    fin >> n;
    for (int i=0;i<n;++i)
    {
        fin >> proc.arrival;
        p.push_back(proc);
    }
    for (int i=0;i<n;++i)
    {
        fin >> p[i].burst;
        p[i].now = p[i].burst;
        coming.push_back(&p[i]);
    }
    
    fin >> quantum1;
    fin >> quantum2;
   
    t = 0;
    while(!coming.empty() || !Hpriority.empty() || !Mpriority.empty() || !Lpriority.empty() )
    {
        if (!coming.empty() && t>=coming.front()->arrival)
        {
            Hpriority.push_back(coming.front());
            coming.pop_front();
        }

        if (!Hpriority.empty())
        {
            if(Hpriority.front()->now==(Hpriority.front()->burst-quantum1))
            {
                Mpriority.push_back(Hpriority.front());
                Hpriority.pop_front();
            }
            if (Hpriority.empty())
                continue;
            --Hpriority.front()->now;
            if(!Hpriority.front()->now)
            {
                Hpriority.front()->turn = (t+1) - Hpriority.front()->arrival;
                Hpriority.front()->wait = Hpriority.front()->turn - Hpriority.front()->burst;
                Hpriority.pop_front();
            }
        }
        else if (!Mpriority.empty())
        {
            if (Mpriority.front()->now==(Mpriority.front()->burst-(quantum1+quantum2)))
            {
                if (Lpriority.empty() || Lpriority.front()->now>Mpriority.front()->now)
                    Lpriority.push_front(Mpriority.front());
                else
                {
                    deque<process*>::iterator it;
                    for (it=Lpriority.begin();it!=Lpriority.end();++it)
                    {
                        if((*it)->now>Mpriority.front()->now)
                        {
                            Lpriority.insert(it,Mpriority.front());
                            break;
                        }
                    }
                    if (Lpriority.back()->now<Mpriority.front()->now)
                        Lpriority.push_back(Mpriority.front());
                }
                Mpriority.pop_front();
            }
            if (Mpriority.empty())
                continue;
            --Mpriority.front()->now;
            if (!Mpriority.front()->now)
            {
                Mpriority.front()->turn = (t+1) - Mpriority.front()->arrival;
                Mpriority.front()->wait = Mpriority.front()->turn - Mpriority.front()->burst;
                Mpriority.pop_front();
            }
        }
        else if (!Lpriority.empty())
        {
            --Lpriority.front()->now;
            if(!Lpriority.front()->now)
            {
                Lpriority.front()->turn = (t+1) - Lpriority.front()->arrival;
                Lpriority.front()->wait = Lpriority.front()->turn - Lpriority.front()->burst;
                Lpriority.pop_front();
            }
        }
        ++t;
    }

    cout << "Process   Waiting Time   Turnaround Time" << endl;
    for(int i=0;i<n;++i)
    {
        cout << "p[" << i+1 << "]";
        cout.width(6);
        cout << left << " ";
        cout.width(15);
        cout << left << p[i].wait;
        cout << left << p[i].turn;
        cout << endl;
        ave_wait += p[i].wait;
        ave_turn += p[i].turn;
    }
    cout << endl;
    cout << "Average waiting time :" << ave_wait/n << endl;
    cout << "Average turnaround time :" << ave_turn/n << endl;


    return 0;
}
