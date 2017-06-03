// Simulation for channel utilization
// joseph chen 20170528

#include<time.h>
#include<math.h>
#include <windows.h>// Windows function (GetTickCount, ....)
#include<iostream>
#include<fstream>
#include <vector>
#include <queue>
using namespace std;
#define MAX_TEST 500   // 1000000   handle the max event pair <arrival,departure>
#define MAX_CHANNEL 80000
enum {ARRIVAL = 1, DEPARTURE = 2};
enum {OFF=0,ON=1 };
char filename[]="arrival.txt";
char filename2[]="service.txt";
fstream fp;
fstream fp2;


//Event and eventComp  
class Event { 
public: 
	Event(){ event_type=0;}
    int event_type; // 1.ARRIVAL   2.DEPARTURE
	double timeStamp;
};

class Channel { 
public: 
	Channel(){ state=OFF;}
    int state; // OFF=0,ON=1
	double timeStamp;
};


//Function list
double U(); ////Uniform distribution (continuous)
double E(double r);//Exponential distribution
double G(double r, double k);//Gamma distribution
double erlangb(double r, int c);// Erlang-B formula
void sim(double arrival_rate, double service_rate, int channel);


//Array priority queue
Event* arr[300000];
int pq_ptr = 0;
Channel ch;

void push(Event* my_e )
{
	arr[pq_ptr] = my_e;
	for(int kn= pq_ptr - 1; kn>=0 ; kn--){
		if(arr[kn]->timeStamp < arr[kn + 1 ]->timeStamp){
			Event* tempss=arr[kn];
			arr[kn]=arr[kn + 1 ];
			arr[kn + 1 ] = tempss;
		}else { break; }
	}
	pq_ptr++;
}

//Simulate
void sim(double arrival_rate, double service_rate, int channel)
{
	//Initialize
	double ts=0.0;
	int Number_of_Case = 0;
	int Bound = MAX_TEST;
	
	Event *gen_event, *list_event;
	int N_arrival = 0;
	int N_departure = 0;
	int N_blocking =0;
    double ts_0;
	

	//First event
	gen_event = new Event;
	gen_event->event_type = ARRIVAL;
	gen_event->timeStamp = 0.0;
	

	push( gen_event );

	//Start to simulate
	while(Number_of_Case <= Bound )
	{

		list_event= arr[ pq_ptr - 1 ];
		pq_ptr--;
        ts_0=ts;
		ts = list_event->timeStamp;
		
        fp << ch.state <<'\t'<< list_event->event_type << '\t' << ts << '\t' << ts-ts_0 <<endl;
      
		switch (list_event->event_type ){
			case ARRIVAL:  //primary user arrives 
                ch.state=ON;
				Number_of_Case++;
				N_arrival++;
				
				//if(channel>0)
			//	{
				//	channel--;
					gen_event = new Event;
					gen_event->event_type = DEPARTURE;
				    gen_event->timeStamp = ts + E(service_rate);
					push( gen_event );
			//	}
		     //	else{ N_blocking++;}

				break;
			case DEPARTURE:  // primary user leaves 
                ch.state=OFF;
				//channel++;
				N_departure++;
				gen_event = new Event;
				gen_event->event_type = ARRIVAL; 
				gen_event->timeStamp = ts + E(arrival_rate);  //generate next arrival event
			    push( gen_event );

				break;
		}
		delete list_event;
	}

	// Calculate the probability
	cout << "N_arrival=" << N_arrival  << endl;
	cout << "N_departure=" << N_departure  << endl;
	cout << "N_blocking=" << N_blocking  << endl;
	cout << "blocking rate=" << 1.0*N_blocking/N_arrival << endl;
	

}
//------------------------
// Main Function 
//-------------------------
int main()
{
    srand(time(NULL));
    //for primary user parameter
	double arrival = 200.0;  // average arrival rate: (period:0.005s)
	double service = 5.0;  // average dwell time: 0.2s
 	int channel = MAX_CHANNEL;  //total number of available channels

	DWORD stick, etick;
	time_t start, finish;

	time(&start);  stick = GetTickCount();
	printf("Test beging...\n");
	
	
#if 0	
   fp.open(filename, ios::out);
   long i;
   double Tn;
   for(i=0;i<10000;i++) {
    Tn=E(service);  
    cout << i << '\t' << Tn <<endl;
    fp << i << '\t' << Tn <<endl;
   }
#else 
	
	//write to file 
	fp.open(filename, ios::out);
    if(!fp){
        cout<<"Fail to open file: "<<filename<<endl;
    }
    fp2.open(filename2, ios::out);
    if(!fp2){
        cout<<"Fail to open file: "<<filename2<<endl;
    }
    cout<<"Starting Simulation"<<endl;
  

	sim(arrival, service, channel);
#endif
	time(&finish); etick = GetTickCount();
	printf("Elapsed second(s)=%lf, Elapsed tick(s)=%u\n",  difftime(finish, start), etick-stick);
	cout << "Erlang-B formula:" << erlangb(arrival/service,channel) << endl;
    fp.close();
    fp2.close();
	system("PAUSE");
	return 0;
}




//------------------------------
// Utility function
//-------------------------------
//Uniform distribution (continuous)
double U()
{
	int temp = 0;
	while( temp==0 ) {temp=rand();}
	return 1.0*temp/RAND_MAX; 
}
//Exponential distribution
double E(double r)
{
	return -1.0*log(U())/r;
}
//Gamma distribution
double G(double r, double k)
{ 
	// from http://en.wikipedia.org/wiki/Gamma_distribution
	double m, v3m_2, v3m_1, v3m, xi, eta, K, delta = modf (k , &K), e=2.71828183;
	step1: m=1;
	step2: 
		v3m_2 = U(); 
		v3m_1 = U(); 
		v3m = U();
	step3: if( v3m_2 <= e/(e+delta)){goto step4;}else{goto step5;}
    step4: xi=pow(v3m_1, 1.0/delta);
			  eta =v3m*pow( xi , delta - 1.0);
			  goto step6;
	step5: xi = 1.0 - log(v3m_1);
			  eta = v3m*exp(-1.0*xi);
	step6: if( eta > pow(xi,delta-1.0)*exp(-1.0*xi) ){ m++; goto step2;}
	step7: for(int i=1;i<=K;i++){ xi -= log(U()); }
			  xi *=r;
	return xi;
}
//Erlang-B formula
double erlangb(double r, int c)
{
	double temp = 1.0;
	for(int k=1;k<=c;k++)
	{
		temp = (r*temp)/(1.0*k+r*temp);
	}
	return temp;
}
 


