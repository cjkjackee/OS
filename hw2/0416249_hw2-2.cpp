#include <iostream>
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
    double wait;
    double turn;
};

int main ()
{
    int n;
    int t;
    double ave_wait=0, ave_turn=0;
    process p;
    vector<process> proc;
    deque<process*> q, waiting;

    cin >> n;
    for (int i=0;i<n;++i)
    {
        cin >> p.arrival;
        proc.push_back(p);
    }
    for (int i=0;i<n;++i)
    {
        cin >> proc[i].burst;
        q.push_back(&proc[i]);
    }

    cout << "Process   Waiting Time   Turnaround Time" << endl;

    t = 0;
    while(1)
    {
        while(!q.empty() && t>=q.front()->arrival)
        {
            if (waiting.empty() || waiting.front()->burst<q.front()->burst)
            {
                waiting.push_back(q.front());
                q.pop_front();
            }
            else
            {
                waiting.push_front(q.front());
                q.pop_front();
            }
        }

        waiting.front()->wait = t - waiting.front()->arrival;
        t += waiting.front()->burst;
        waiting.front()->turn = waiting.front()->wait + waiting.front()->burst;
        waiting.pop_front();

        if (q.empty() && waiting.empty())
            break;
    }

    for(int i=0;i<n;++i)
    {
        cout << "p[" << i+1 << "]";
        cout.width(6);
        cout << left << " ";
        cout.width(15);
        cout << left << proc[i].wait;
        cout << left << proc[i].turn;
        cout << endl;
        ave_wait += proc[i].wait;
        ave_turn += proc[i].turn;
    }
    cout << endl;
    cout << "Average waiting time :" << ave_wait/n << endl;
    cout << "Average turnaround time :" << ave_turn/n << endl;
    
    return 0;
}
