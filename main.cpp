#include <bits/stdc++.h>
using namespace std;
float inf = numeric_limits<float>::infinity();
using ll = long long;

const double end_time = 50000;
//客の到着率(人/時)
const double lambda = 100;
//待ち室容量
const int K = 1000;
const double TAXI_NUM = 65;
const double velocity = 20;
const double radius = 7;
const double set_time = 3.0;
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
    mt19937 mt{ std::random_device{}() };
    //random_device rd;
    //default_random_engine eng(rd());
    uniform_real_distribution<double> distr(Min, Max);
    double g = distr(mt);
    return g;
}

//到着間隔
double Arrival_interval() {
    double g = my_rand(0, 1);
    double tau = - log(1 - g) / lambda;
    return tau;
}

//円形領域内の座標をランダムに取得
pair<double, double> coordinate() {
    double r = radius * sqrt(my_rand(0, 1));
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
    double service_time_sum = 0.0;
    double Length_sum = 0.0;
    double pop_cnt = 0;
    
    cout << "1 : First Come First Served" << endl;
    vector<bool> p_flag(4, 1);
    cout << "Progress is 0%";
    
    //タクシーの位置を初期化
    if (TAXI_NUM) {
        taxi_pos.at(0) = make_pair(0, 0);
    } else {
        for (int i = 0; i < TAXI_NUM; i++) {
            taxi_pos.at(i) = coordinate();
        }
    }
    
    
    //1人目登場
    double current_time = Arrival_interval();
    event.push(make_tuple(current_time, 0, 0));
    
    while (get<0>(event.top()) < end_time) {
        //客発生
        if (get<1>(event.top()) == 0) {
            pair<double, double> passenger = coordinate();
            //空きタクシーがなければ待ち室に並ばせる
            if (taxi_que.empty()) {
                if (que.size() < K) que.push(current_time);
            } else {
                //ピックアップ時間を計算
                double pickup_time = cal_dst(taxi_pos.at(taxi_que.front()), passenger) / velocity;
                taxi_pos.at(taxi_que.front()) = passenger;
                event.push(make_tuple(current_time + pickup_time, 1, taxi_que.front()));
                taxi_que.pop();
                
                if (get<0>(event.top()) >= end_time / set_time) {
                    //サービス時間加算
                    service_time_sum += pickup_time;
                }
            }

            //次の客発生時刻をイベントにpush
            current_time += Arrival_interval();
            event.push(make_tuple(current_time, 0, -1));
            
        } else if (get<1>(event.top()) == 1) {  //乗車
            //降車時間を計算
            pair<double, double> destination = coordinate();
            double service_time = cal_dst(taxi_pos.at(get<2>(event.top())), destination) / velocity;
            //サービスタイム加算
            if (get<0>(event.top()) >= end_time / set_time) {
                service_time_sum += service_time;
            }
            //タクシーの座標を降車位置に
            taxi_pos.at(get<2>(event.top())) = destination;
            //降車イベントをpush
            event.push(make_tuple(get<0>(event.top()) + service_time, 2, get<2>(event.top())));
            
        } else if (get<1>(event.top()) == 2) {  //降車
            if (!que.empty()) {
                //配車，次の客のピックアップ時間を計算
                pair<double, double> passenger = coordinate();
                
                //信頼性検証用
                if (TAXI_NUM == 1) {
                    taxi_pos.at(get<2>(event.top())) = make_pair(0, 0);
                }
                
                double pickup_time = cal_dst(taxi_pos.at(get<2>(event.top())), passenger) / velocity;
                event.push(make_tuple(get<0>(event.top()) + pickup_time, 1, get<2>(event.top())));
                //タクシーの座標を迎えに行く客の座標にする
                taxi_pos.at(get<2>(event.top())) = passenger;
                
                //初めの1/3のデータは捨てる
                if (get<0>(event.top()) >= end_time / set_time) {
                    //行列長を足す
                    Length_sum += que.size() - 1;
                    //待ち時間を足す
                    wait_time_sum += get<0>(event.top()) - que.front();
                    //サービス時間を足す
                    service_time_sum += pickup_time;
                }
                que.pop();
            } else {
                //タクシーの状態を空車に
                taxi_que.push(get<2>(event.top()));
                if (TAXI_NUM) taxi_pos.at(get<2>(event.top())) = make_pair(0, 0);
            }
            //サービスを終えた客をカウント
            if (get<0>(event.top()) >= end_time / set_time) {
                pop_cnt += 1;
            }
            
        }
        
        //進捗状況を表示
        double progress = get<0>(event.top()) / end_time;
        if (progress > 0.8 && p_flag.at(3)) {cout << "...80%"; p_flag.at(3) = false;}
        else if (progress > 0.6 && p_flag.at(2)) {cout << "...60%"; p_flag.at(2) = false;}
        else if (progress > 0.4 && p_flag.at(1)) {cout << "...40%"; p_flag.at(1) = false;}
        else if (progress > 0.2 && p_flag.at(0)) {cout << "...20%"; p_flag.at(0) = false;}
        
        event.pop();
    }
    
    cout << "...100%" << endl;
    
    cout << "Wait time average = " << wait_time_sum / pop_cnt << endl;
    cout << "Length average = " << Length_sum / pop_cnt << endl;
    cout << "Service time average = " << service_time_sum / pop_cnt << endl;
    cout << "roh = " << lambda * service_time_sum / pop_cnt / (double)TAXI_NUM << endl;
    cout << endl;
    
}

double now = 0;
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

    double service_time_sum = 0.0;
    double wait_time_sum = 0.0;
    double Length_sum = 0.0;
    double pop_cnt = 0;
    
    cout << "2 : Closest order" << endl;
    vector<bool> p_flag(4, 1);
    cout << "Progress is 0%";
    
    //タクシーの位置を初期化
    for (int i = 0; i < TAXI_NUM; i++) {
        taxi_pos.at(i) = coordinate();
    }
    
    //1人目登場
    double current_time = Arrival_interval();
    event.push(make_tuple(current_time, 0, 1));
    
    while (get<0>(event.top()) < end_time) {
        //客発生
        if (get<1>(event.top()) == 0) {
            pair<double, double> passenger = coordinate();
            //空きタクシーがなければ待ち室に並ばせる
            if (taxi_list.empty()) {
                if (pas_vec.size() < K) pas_vec.push_back(make_pair(current_time, passenger));
                
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
                if (get<0>(event.top()) >= end_time / set_time) {
                    //サービス時間加算
                    service_time_sum += pickup_time;
                }
            }

            //次の客発生時刻をイベントにpush
            current_time += Arrival_interval();
            event.push(make_tuple(current_time, 0, -1));
            
        } else if (get<1>(event.top()) == 1) {  //乗車
            //降車時間を計算
            pair<double, double> destination = coordinate();
            double service_time = cal_dst(taxi_pos.at(get<2>(event.top())), destination) / velocity;
            if (get<0>(event.top()) >= end_time / set_time) {
                service_time_sum += service_time;
            }
            //タクシーの座標を降車位置に
            taxi_pos.at(get<2>(event.top())) = destination;
            //降車イベントをpush
            event.push(make_tuple(get<0>(event.top()) + service_time, 2, get<2>(event.top())));
            
        } else if (get<1>(event.top()) == 2) {  //降車
            if (!pas_vec.empty()) {
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
                
                //初めの1/3のデータは捨てる
                if (get<0>(event.top()) >= end_time / set_time) {
                    //行列長を足す
                    Length_sum += pas_vec.size() - 1;
                    //待ち時間を足す
                    wait_time_sum += get<0>(event.top()) - pas_vec.at(num).first;
                    //サービス時間を足す
                    service_time_sum += pickup_time;
                }
                //配列の要素削除は時間がかかるため，末尾を取り出したデータの格納先にコピーしてから末尾削除(定数時間)
                pas_vec.at(num) = pas_vec.back();
                pas_vec.pop_back();
            } else {
                //空きタクシーをリストに追加
                taxi_list.push_back(get<2>(event.top()));
            }
            
            if (get<0>(event.top()) >= end_time / set_time) {
                //サービスを終えた客をカウント
                pop_cnt += 1;
            }
        }
        
        //進捗状況を表示
        double progress = get<0>(event.top()) / end_time;
        if (progress > 0.8 && p_flag.at(3)) {cout << "...80%"; p_flag.at(3) = false;}
        else if (progress > 0.6 && p_flag.at(2)) {cout << "...60%"; p_flag.at(2) = false;}
        else if (progress > 0.4 && p_flag.at(1)) {cout << "...40%"; p_flag.at(1) = false;}
        else if (progress > 0.2 && p_flag.at(0)) {cout << "...20%"; p_flag.at(0) = false;}
        
        now = get<0>(event.top());
        event.pop();
    }
    cout << now << endl;
    cout << "100%" << endl;
    
    cout << "Wait time average = " << wait_time_sum / pop_cnt << endl;
    cout << "Length average = " << Length_sum / pop_cnt << endl;
    cout << "Service time average = " << service_time_sum / pop_cnt << endl;
    cout << "roh = " << lambda * service_time_sum / pop_cnt / (double)TAXI_NUM << endl;
    
}



int main() {
//    cout << "Which algorithm ?" << endl;
//    cout << "1 : First Come First Served" << endl;
//    cout << "2 : Closest order" << endl;
//    cout << "Put number 1 or 2 :";
//    int num; cin >> num;
//
//    if (num == 1) fcfs();
//    else closest();
    fcfs();
    closest();

}
