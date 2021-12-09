#include <bits/stdc++.h>

using namespace std;
float inf = numeric_limits<float>::infinity();
using ll = long long;
//シミュレーション時間
const double end_time = 500000;
//客の到着率(人/時)
double lambda;
//待ち室容量
const int K = inf;
const double velocity = 20;
const double radius = 7.0;
const double set_time = 7.0;
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
void fcfs(int TAXI_NUM) {
    string filename = to_string(TAXI_NUM) + ".txt";
    //出力ファイルを開く
    ofstream outputfile;
    outputfile.open(filename);
    
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
    double rjc_cnt = 0;
    double arv_cnt = 0;
    
    cout << "1 : First Come First Served" << " " << TAXI_NUM << endl;
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
    
    double a = 0, sum = 0;
    while (get<0>(event.top()) < end_time) {
        //客発生
        if (get<1>(event.top()) == 0) {
            arv_cnt += 1;
            pair<double, double> passenger = coordinate();
            //空きタクシーがなければ待ち室に並ばせる
            if (taxi_que.empty()) {
                if (que.size() < K) que.push(current_time);
                else rjc_cnt += 1;
            } else {
                //ピックアップ時間を計算
                double pickup_time = cal_dst(taxi_pos.at(taxi_que.front()), passenger) / velocity;
                taxi_pos.at(taxi_que.front()) = passenger;
                event.push(make_tuple(current_time + pickup_time, 1, taxi_que.front()));
                taxi_que.pop();
                
                if (get<0>(event.top()) >= end_time / set_time) {
                    //サービス時間加算
                    service_time_sum += pickup_time;
                    a += pickup_time;
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
                a += service_time;
                sum += a*a;
                a = 0;
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
                    double WT = get<0>(event.top()) - que.front();
                    wait_time_sum += WT;
                    //待ち時間をファイルに出力
                    outputfile << WT << endl;
                    //サービス時間を足す
                    service_time_sum += pickup_time;
                    a += pickup_time;
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
    //結果を出力
    cout << "Wait time average = " << wait_time_sum / pop_cnt << endl;
    cout << "Length average = " << Length_sum / pop_cnt << endl;
    cout << "Service time average = " << service_time_sum / pop_cnt << endl;
    cout << "roh = " << lambda * service_time_sum / pop_cnt / (double)TAXI_NUM << endl;
    cout << "Rejection rate = " << rjc_cnt / arv_cnt << endl;
    cout << "Service time square = " << sum / pop_cnt << endl;
    
    cout << endl;
    outputfile.close();
    
}





//Shortest processing time first
void spt(int TAXI_NUM) {
    string filename = to_string(TAXI_NUM) + "s.txt";
    //出力ファイルを開く
    ofstream outputfile;
    outputfile.open(filename);
    
    //客の発生時刻と発生座標，客の現在地から目的地までの距離を持つ配列，仮待ち行列 : 一番近い客はタクシーによって異なるためqueueもp_queも使えない．一番近い客の情報へのアクセス(ランダムアクセス)が速い配列を採用
    vector<tuple<double, pair<double, double>, pair<double, double>>> pas_vec;
    //時間，イベントの種類，タクシー番号を保持して時間の早い順に取り出す 0:客発生 1:乗車 2:降車
    priority_queue<tuple<double, int, int>, vector<tuple<double, int, int>>, greater<tuple<double, int, int>>> event;
    vector<pair<double, double>> taxi_pos(TAXI_NUM);
    //空きタクシーのリスト:空いている時，客から一番近いタクシーをリストから削除(客が乗車)する．削除が速いリストを採用
    list<int> taxi_list;
    for (int i = 0; i < TAXI_NUM; i++) taxi_list.push_back(i);

    double service_time_sum = 0.0;
    double wait_time_sum = 0.0;
    double Length_sum = 0.0;
    double pop_cnt = 0;
    double rjc_cnt = 0;
    double arv_cnt = 0;
    
    cout << "2 : Shortest processing time first" << " " << TAXI_NUM << endl;
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
            arv_cnt += 1;
            pair<double, double> passenger = coordinate();
            pair<double, double> destination = coordinate();
            
            //空きタクシーがなければ待ち室に並ばせる
            if (taxi_list.empty()) {
                if (pas_vec.size() < K) pas_vec.push_back(make_tuple(current_time, passenger, destination));
                else rjc_cnt += 1;
                
            } else {
                //客から最短距離のタクシーを探索
                double min_dst = inf;
                auto num = taxi_list.begin();
                for (auto x = num; x != taxi_list.end(); x++) {
                    double dst_service = cal_dst(passenger, taxi_pos.at(*x));
                    if (dst_service < min_dst) {
                        min_dst = dst_service;
                        num = x;
                    }
                }
                //サービス時間を計算
                double distance = cal_dst(passenger, destination);
                double pickup_time = (min_dst + distance) / velocity;
                taxi_pos.at(*num) = destination;
                //降車処理を飛ばす
                event.push(make_tuple(current_time + pickup_time, 2, *num));
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
//            pair<double, double> destination = coordinate();
//            double service_time = cal_dst(taxi_pos.at(get<2>(event.top())), destination) / velocity;
//            if (get<0>(event.top()) >= end_time / set_time) {
//                service_time_sum += service_time;
//            }
//            //タクシーの座標を降車位置に
//            taxi_pos.at(get<2>(event.top())) = destination;
//            //降車イベントをpush
//            event.push(make_tuple(get<0>(event.top()) + service_time, 2, get<2>(event.top())));
            
        } else if (get<1>(event.top()) == 2) {  //降車
            if (!pas_vec.empty()) {
                //サービス時間最短の客を探索
                double min_dst = inf;
                int num = 0;
                for (int j = 0; j < pas_vec.size(); j++) {
                    double dst_service = cal_dst(get<1>(pas_vec.at(j)), taxi_pos.at(get<2>(event.top()))) + cal_dst(get<1>(pas_vec.at(j)), get<2>(pas_vec.at(j)));
                    if (dst_service < min_dst) {
                        min_dst = dst_service;
                        num = j;
                    }
                }
                //次の客のサービス時間を計算
                double service_time = min_dst / velocity;
                //タクシーの座標を客の目的地の座標にする
                taxi_pos.at(get<2>(event.top())) = get<2>(pas_vec.at(num));
                event.push(make_tuple(current_time + service_time, 2, get<2>(event.top())));
                
                //初めの1/3のデータは捨てる
                if (get<0>(event.top()) >= end_time / set_time) {
                    //行列長を足す
                    Length_sum += pas_vec.size() - 1;
                    //待ち時間を足す
                    double WT = get<0>(event.top()) - get<0>(pas_vec.at(num));
                    wait_time_sum += WT;
                    //待ち時間をファイルに出力
                    outputfile << WT << endl;
                    //サービス時間を足す
                    service_time_sum += service_time;
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
        
        //now = get<0>(event.top());
        event.pop();
    }
    //cout << now << endl;
    cout << "...100%" << endl;
    //結果を出力
    cout << "Wait time average = " << wait_time_sum / pop_cnt << endl;
    cout << "Length average = " << Length_sum / pop_cnt << endl;
    cout << "Service time average = " << service_time_sum / pop_cnt << endl;
    cout << "roh = " << lambda * service_time_sum / pop_cnt / (double)TAXI_NUM << endl;
    cout << "Rejection rate = " << rjc_cnt / arv_cnt << endl;
    cout << "Length error = " << Length_sum / pop_cnt - (1 - rjc_cnt / arv_cnt) * lambda * wait_time_sum / pop_cnt << endl;
    
    outputfile.close();
    
}





//Nearest first
void nearest(int TAXI_NUM) {
    string filename = to_string(TAXI_NUM) + "c.txt";
    //出力ファイルを開く
    ofstream outputfile;
    outputfile.open(filename);
    
    //客の発生時刻と座標を持つ配列，仮待ち行列 : 一番近い客はタクシーによって異なるためqueueもp_queも使えない．一番近い客の情報へのアクセス(ランダムアクセス)が速い配列を採用
    vector<pair<double, pair<double, double>>> pas_vec;
    //時間，イベントの種類，タクシー番号を保持して時間の早い順に取り出す 0:客発生 1:乗車 2:降車
    priority_queue<tuple<double, int, int>, vector<tuple<double, int, int>>, greater<tuple<double, int, int>>> event;
    vector<pair<double, double>> taxi_pos(TAXI_NUM);
    //空きタクシーのリスト:空いている時，客から一番近いタクシーをリストから削除(客が乗車)する．削除が速いリストを採用
    list<int> taxi_list;
    for (int i = 0; i < TAXI_NUM; i++) taxi_list.push_back(i);

    double service_time_sum = 0.0;
    double wait_time_sum = 0.0;
    double Length_sum = 0.0;
    double pop_cnt = 0;
    double rjc_cnt = 0;
    double arv_cnt = 0;
    
    cout << "3 : Nearest first" << " " << TAXI_NUM << endl;
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
            arv_cnt += 1;
            pair<double, double> passenger = coordinate();
            //空きタクシーがなければ待ち室に並ばせる
            if (taxi_list.empty()) {
                if (pas_vec.size() < K) pas_vec.push_back(make_pair(current_time, passenger));
                else rjc_cnt += 1;
                
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
                    double WT = get<0>(event.top()) - pas_vec.at(num).first;
                    wait_time_sum += WT;
                    //待ち時間をファイルに出力
                    outputfile << WT << endl;
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
        
        //now = get<0>(event.top());
        event.pop();
    }
    //cout << now << endl;
    cout << "...100%" << endl;
    //結果を出力
    cout << "Wait time average = " << wait_time_sum / pop_cnt << endl;
    cout << "Length average = " << Length_sum / pop_cnt << endl;
    cout << "Service time average = " << service_time_sum / pop_cnt << endl;
    cout << "roh = " << lambda * service_time_sum / pop_cnt / (double)TAXI_NUM << endl;
    cout << "Rejection rate = " << rjc_cnt / arv_cnt << endl;
    cout << "Length error = " << Length_sum / pop_cnt - (1 - rjc_cnt / arv_cnt) * lambda * wait_time_sum / pop_cnt << endl;
    
    outputfile.close();
    
}



int main() {
//    cout << "Which algorithm ?" << endl;
//    cout << "1 : First Come First Served" << endl;
//    cout << "2 : Nearest order" << endl;
//    cout << "3 : SPT << endl;
//    cout << "Put number :";
//    int num; cin >> num;
//
//    if (num == 1) fcfs();
//    else if (num == 2) nearest();
    //cout << "PUT TAXI NUMBER :";
    //int TAXI_NUM; cin >> TAXI_NUM;
    //fcfs(TAXI_NUM);
    int num, lambda_start = 120, lambda_end = 200;
    cout << "Put Taxi num : "; cin >> num; cout << endl;
    for (int j = lambda_start; j <= lambda_end; j+= 20) {
        lambda = j;
        fcfs(num);
        spt(num);
        nearest(num);
    }
    
}
