#include <bits/stdc++.h>
using namespace std;
float inf = numeric_limits<float>::infinity();
using ll = long long;

const int N = 100000;
const double lambda = 100;
const double TAXI_NUM = 13;
const double velocity = 20;
constexpr double PI = 3.14159265358979323846264338;


double urand(){
    double m, a;
    m = RAND_MAX + 1.0;
    a = (rand() + 0.5)/m;
    a = (rand() + a)/m;
    return (rand() + a)/m;
}

//擬似乱数発生
double my_rand(double Min, double Max) {
    random_device rd;
    default_random_engine eng(rd());
    uniform_real_distribution<float> distr(Min, Max);
    double g = distr(eng);
    return g;
}

//到着間隔
double Arrival_interval() {
    double g = my_rand(0, 1);
    double tau = - log(1 - g) / lambda;
    return tau;
}

//座標
pair<double, double> coordinate() {
    double r = sqrt(my_rand(0, 3));
    double theta = my_rand(-PI, PI);
    double x = r * cos(theta);
    double y = r * sin(theta);
    pair<double, double> pos = make_pair(x, y);
    return pos;
}

//2点間の距離を計算
double cal_dst(pair<double, double> pos1, pair<double, double> pos2) {
    return sqrt((pos1.first - pos2.first)*(pos1.first - pos2.first) + (pos1.second - pos2.second)*(pos1.second - pos2.second));
}


//先着順
void fcfs() {
    //時間，イベントの種類，タクシー番号を保持して時間の早い順に取り出す 0:客発生 1:乗車 2:降車
    priority_queue<tuple<double, int, int>, vector<tuple<double, int, int>>, greater<tuple<double, int, int>>> event;
    vector<pair<double, double>> taxi_pos(TAXI_NUM);
    //空きタクシーのキュー
    queue<int> taxi_que;
    for (int i = 0; i < TAXI_NUM; i++) taxi_que.push(i);
    //客の仮想待ち行列
    queue<double> que;
    double wait_time_sum = 0.0;
    double Length_sum = 0.0;
    double pop_cnt = 0;
    
    //タクシーの位置を初期化
    for (int i = 0; i < TAXI_NUM; i++) {
        taxi_pos.at(i) = coordinate();
    }
    
    //1人目登場
    double current_time = Arrival_interval();
    event.push(make_tuple(current_time, 0, 1));
    
    for (int i = 0; i < N; i++) {
        //客発生
        if (get<1>(event.top()) == 0) {
            pair<double, double> passenger = coordinate();
            //空きタクシーがなければ待ち室に並ばせる
            if (taxi_que.empty()) que.push(current_time);
            else {
                //ピックアップ時間を計算
                double pickup_time = cal_dst(taxi_pos.at(taxi_que.front()), passenger) / velocity;
                taxi_pos.at(taxi_que.front()) = passenger;
                event.push(make_tuple(current_time + pickup_time, 1, taxi_que.front()));
                taxi_que.pop();
            }

            event.pop();
            //次の客発生時刻をイベントにpush
            current_time += Arrival_interval();
            event.push(make_tuple(current_time, 0, 1));
            
        } else if (get<1>(event.top()) == 1) {  //乗車
            //降車時間を計算
            pair<double, double> destination = coordinate();
            double service_time = get<0>(event.top()) + cal_dst(taxi_pos.at(get<2>(event.top())), destination) / velocity;
            //タクシーの座標を降車位置に
            taxi_pos.at(get<2>(event.top())) = destination;
            //降車イベントをpush
            event.push(make_tuple(service_time, 2, get<2>(event.top())));
            //乗車イベントをpop
            event.pop();
        } else if (get<1>(event.top()) == 2) {  //降車
            if (!que.empty()) {
                //行列長を足す
                Length_sum += que.size();
                //次の客のピックアップ時間を計算
                pair<double, double> passenger = coordinate();
                double pickup_time = cal_dst(taxi_pos.at(get<2>(event.top())), passenger) / velocity;
                event.push(make_tuple(get<0>(event.top()) + pickup_time, 1, get<2>(event.top())));
                taxi_pos.at(get<2>(event.top())) = passenger;
                //待ち時間を足す
                wait_time_sum += get<0>(event.top()) - que.front();
                que.pop();
                pop_cnt += 1;
            } else {
                //タクシーの状態を空車に
                //taxi_state.at(get<2>(event.top())) = 0;
                taxi_que.push(get<2>(event.top()));
            }
            event.pop();
        }
    }
    
    cout << "Wait time average = " << wait_time_sum / pop_cnt << endl;
    cout << "Length average = " << Length_sum / pop_cnt << endl;
    
    
}


//近接順
void closest() {
    //客の発生時刻と座標を持つ配列，仮待ち行列
    vector<pair<double, pair<double, double>>> pas_vec;
    //時間，イベントの種類，タクシー番号を保持して時間の早い順に取り出す 0:客発生 1:乗車 2:降車
    priority_queue<tuple<double, int, int>, vector<tuple<double, int, int>>, greater<tuple<double, int, int>>> event;
    vector<pair<double, double>> taxi_pos(TAXI_NUM);
    //空きタクシーのリスト
    list<int> taxi_list;
    for (int i = 0; i < TAXI_NUM; i++) taxi_list.push_back(i);

    double wait_time_sum = 0.0;
    double Length_sum = 0.0;
    double pop_cnt = 0;
    
    //タクシーの位置を初期化
    for (int i = 0; i < TAXI_NUM; i++) {
        taxi_pos.at(i) = coordinate();
    }
    
    //1人目登場
    double current_time = Arrival_interval();
    event.push(make_tuple(current_time, 0, 1));
    
    for (int i = 0; i < N; i++) {
        //客発生
        if (get<1>(event.top()) == 0) {
            pair<double, double> passenger = coordinate();
            //空きタクシーがなければ待ち室に並ばせる
            if (taxi_list.empty()) {
                pas_vec.push_back(make_pair(current_time, passenger));
                //que.push(current_time);
            } else {
                //客から最短距離のタクシーを探索
                double min_dst = inf;
                auto num = taxi_list.begin();
                for (auto x = num; x != taxi_list.end(); x++) {
                    if (cal_dst(passenger, taxi_pos.at(*x)) < min_dst) {
                        min_dst = cal_dst(passenger, taxi_pos.at(*x));
                        num = x;
                    }
                }
                //ピックアップ時間を計算
                double pickup_time = min_dst / velocity;
                taxi_pos.at(*num) = passenger;
                event.push(make_tuple(current_time + pickup_time, 1, *num));
                taxi_list.erase(num);
            }

            event.pop();
            //次の客発生時刻をイベントにpush
            current_time += Arrival_interval();
            event.push(make_tuple(current_time, 0, 1));
            
        } else if (get<1>(event.top()) == 1) {  //乗車
            //降車時間を計算
            pair<double, double> destination = coordinate();
            double service_time = get<0>(event.top()) + cal_dst(taxi_pos.at(get<2>(event.top())), destination) / velocity;
            //タクシーの座標を降車位置に
            taxi_pos.at(get<2>(event.top())) = destination;
            //降車イベントをpush
            event.push(make_tuple(service_time, 2, get<2>(event.top())));
            //乗車イベントをpop
            event.pop();
        } else if (get<1>(event.top()) == 2) {  //降車
            if (!pas_vec.empty()) {
                //行列長を足す
                Length_sum += pas_vec.size();
                //タクシーから最短距離の客を探索
                double min_dst = inf;
                int num = 0;
                for (int j = 0; j < pas_vec.size(); j++) {
                    if (cal_dst(pas_vec.at(j).second, taxi_pos.at(get<2>(event.top()))) < min_dst) {
                        min_dst = cal_dst(pas_vec.at(j).second, taxi_pos.at(get<2>(event.top())));
                        num = j;
                    }
                }
                //次の客のピックアップ時間を計算
                double pickup_time = min_dst / velocity;
                //タクシーの座標を迎えに行く客の座標にする
                taxi_pos.at(get<2>(event.top())) = pas_vec.at(num).second;
                event.push(make_tuple(current_time + pickup_time, 1, get<2>(event.top())));
                //待ち時間を足す
                wait_time_sum += get<0>(event.top()) - pas_vec.at(num).first;
                //配列の要素削除は時間がかかるため，末尾を取り出したデータの格納先にコピーしてから末尾削除(定数時間)
                pas_vec.at(num) = pas_vec.back();
                pas_vec.pop_back();
                pop_cnt += 1;
            } else {
                //空きタクシーをリストに追加
                taxi_list.push_back(get<2>(event.top()));
            }
            event.pop();
        }
    }
    
    cout << "Wait time average = " << wait_time_sum / pop_cnt << endl;
    cout << "Length average = " << Length_sum / pop_cnt << endl;
    
    
}



int main() {
    cout << "Which algorithm ?" << endl;
    cout << "1 : First Come First Served" << endl;
    cout << "2 : Closest order" << endl;
    cout << "Put number 1 or 2 :";
    int num; cin >> num;
    
    if (num == 1) fcfs();
    else closest();
}
