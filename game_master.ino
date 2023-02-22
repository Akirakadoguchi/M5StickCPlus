#include <M5StickCPlus.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
String my_name = "Master";
String his_name = "Slave";

#define area_num 5
#define area_l 20
#define road_l 2
#define w 135
#define w1 19
#define w2 2
#define w3 112
#define w4 2
#define h 240
#define h1 2
#define h2 112
#define h3 2
#define h4 8
#define h5 2
#define h6 112
#define h7 2

uint16_t know_wall_color;
uint16_t know_area_color;
uint16_t unknown_wall_color;
uint16_t unknown_area_color;
uint16_t my_area_color;
uint16_t my_wall_color;
uint16_t title_str_color;
uint16_t frame_color;
uint16_t marker_color;
uint16_t finish_color;
uint16_t winner_color;
uint16_t loser_color;
uint16_t back_color;

void set_color(){
  know_wall_color = CYAN;
  know_area_color = BLUE;
  unknown_wall_color = BLACK;
  unknown_area_color = BLACK;
  my_area_color = RED;
  my_wall_color = GREEN;
  title_str_color = WHITE;
  frame_color = BLUE;
  marker_color = GREEN;
  finish_color = BLUE;
  winner_color = RED;
  loser_color = BLUE;
  back_color = BLACK;
}

int reaction;

/*
XXX_pose:{index_x, index_y, direction}
direction:
*/
int my_pose[3]    = {-1, -1, 0};
int enemy_pose[3] = {-1, -1, 0};

/*
XX_sg:{start_x, start_y, goal_x, goal_y}
*/
int my_sg[4]      = {-1, -1, -1, -1};
int enemy_sg[4]   = {-1, -1, -1, -1};
int add_y1 = 4, add_x1 = 4, add_x2 = 2;
int ver_wall_me[area_num][area_num + 1];
int hor_wall_me[area_num + 1][area_num];
int ver_wall_enemy[area_num][area_num + 1];
int hor_wall_enemy[area_num + 1][area_num];
int parity_map[area_num][area_num];
int my_area_map[area_num][area_num];
int enemy_area_map[area_num][area_num];

void initialize_wall() {
  for (int five_i = 0; five_i < area_num; five_i++) {
    for (int six_i = 0; six_i < area_num + 1; six_i++) {
      ver_wall_me[five_i][six_i] = 0;
      hor_wall_me[six_i][five_i] = 0;
      ver_wall_enemy[five_i][six_i] = 0;
      hor_wall_enemy[six_i][five_i] = 0;
      if (six_i!=5){
        my_area_map[six_i][five_i] = 0;
        enemy_area_map[six_i][five_i] = 0;
      }
    }
  }
}

void idx2px(int index_y, int index_x, bool true_me, int *left_py, int *left_px){
  *left_py += h1;
  *left_px += w1 + w2;
  if (true_me){*left_py += h2 + h3 + h4 + h5;}
  *left_py += index_y * (area_l + road_l);
  *left_px += index_x * (area_l + road_l);
}

void change_color(int left_py, int left_px, int height, int width, uint16_t color) {
  int new_px, new_py;
  for (int h_i = 0; h_i < height; h_i++) {
    for (int w_i = 0; w_i < width; w_i++) {
      new_px = left_px + w_i, new_py = left_py + h_i;
      M5.Lcd.drawPixel(new_px, new_py, color);
    }
  }
}

void draw_area(int index_y, int index_x, int true_me, bool time_attack) {
  int width, height;
  int left_py=0, left_px=0;
  uint16_t color;
  if (true_me==1){
    if (time_attack){my_area_map[index_y][index_x] = 1;}
    else            {my_area_map[index_y][index_x] = 0;}
    if      (my_area_map[index_y][index_x]==0){color = my_area_color;}
    else if (my_area_map[index_y][index_x]==1){color = know_area_color;}
    else if (my_area_map[index_y][index_x]==2){color = back_color;}
  }
  else if (true_me==0){
    if (time_attack){enemy_area_map[index_y][index_x] = 1;}
    else            {enemy_area_map[index_y][index_x] = 0;}
    if      (enemy_area_map[index_y][index_x]==0){color = my_area_color;}
    else if (enemy_area_map[index_y][index_x]==1){color = know_area_color;}
    else if (enemy_area_map[index_y][index_x]==2){color = back_color;}
  }
  else if (true_me==2){
    if      (parity_map[index_y][index_x]==0){color = my_area_color;}
    else if (parity_map[index_y][index_x]==1){color = know_area_color;}
    else if (parity_map[index_y][index_x]==2){color = know_area_color;}
    true_me = 0;
  }
    
  idx2px(index_y, index_x, true_me, &left_py, &left_px);
  left_py += road_l;
  left_px += road_l;
  
  change_color(left_py, left_px, area_l, area_l, color);
}

void draw_wall(int index_y, int index_x, bool true_me, bool true_hor, bool time_attack) {
  int width, height;
  int left_py=0, left_px=0;
  uint16_t color;
  bool flag;
  
  idx2px(index_y, index_x, true_me, &left_py, &left_px);
  if (true_hor) {
    width  = area_l, height = road_l;
    left_px += road_l;
    if (true_me&&hor_wall_me[index_y][index_x]==1){color = my_wall_color;}
    else{color = unknown_wall_color;}
  }
  else {
    width  = road_l, height = area_l;
    left_py += road_l;
    if (true_me&&ver_wall_me[index_y][index_x]==1){color = my_wall_color;}
    else{color = unknown_wall_color;}
  }
  
  if (time_attack){
    change_wall_val(true_me, index_y, index_x, 0, 2);
    color = know_wall_color;  
  }
  change_color(left_py, left_px, height, width, color);
}

void draw_one_wall(int index_y, int index_x, int d, bool true_me, bool time_track){
  if (d == 0){draw_wall(index_y, index_x, true_me, true, time_track);}
  if (d == 1){draw_wall(index_y, index_x, true_me, false, time_track);}
  if (d == 2){draw_wall(index_y+1, index_x, true_me, true, time_track);}
  if (d == 3){draw_wall(index_y, index_x+1, true_me, false, time_track);}
}

void draw_char(int index_y, int index_x, bool true_me, String str) {
  uint16_t str_back_color;
  if (true_me){
    if(my_area_map[index_y][index_x]==0){str_back_color = my_area_color;}
    else{str_back_color = know_area_color;}
  }
  else{
    if(enemy_area_map[index_y][index_x]==0){str_back_color = my_area_color;}
    else{str_back_color = know_area_color;}  
  }
  M5.Lcd.setTextColor(marker_color, str_back_color);

  int left_py=0, left_px=0;
  idx2px(index_y, index_x, true_me, &left_py, &left_px);
  left_py += road_l + 2;
  left_px += road_l + 5;
  
  M5.Lcd.drawString(str, left_px, left_py, 2);
}

void draw_triangle(int index_y, int index_x, int d, bool true_me) {
  int left_py=0, left_px=0;
  idx2px(index_y, index_x, true_me, &left_py, &left_px);
  left_py += road_l;
  left_px += road_l;

  int ax = left_px, bx = left_px, cx = left_px;
  int ay = left_py, by = left_py, cy = left_py;
  int A = 7, B = int(area_l / 2), C = 15, D = 3, E = area_l - D;
  if (d == 0) {
    ay += A, ax += B;
    by += C, bx += D;
    cy += C, cx += E;
  }
  else if (d == 1) {
    ay += B, ax += A;
    by += D, bx += C;
    cy += E, cx += C;
  }
  else if (d == 2) {
    ay += area_l - A, ax += B;
    by += area_l - C, bx += D;
    cy += area_l - C, cx += E;
  }
  else if (d == 3) {
    ay += B, ax += area_l - A;
    by += D, bx += area_l - C;
    cy += E, cx += area_l - C;
  }

  if (true_me){my_area_map[index_y][index_x] = 2;}
  else        {enemy_area_map[index_y][index_x] = 2;}
  draw_area(index_y, index_x, true_me, true);
  M5.Lcd.fillTriangle(ax, ay, bx, by, cx, cy, marker_color);
}

void set_area(int true_me) {
  for (int h_i = 0; h_i < area_num; h_i++) {
    for (int w_i = 0; w_i < area_num; w_i++) {draw_area(h_i, w_i, true_me, false);}
  }
}

void set_wall() {
  for (int five_i = 0; five_i < area_num; five_i++) {
    for (int six_i = 0; six_i < area_num + 1; six_i++) {
      draw_wall(six_i, five_i, true, true, false);
      draw_wall(five_i, six_i, true, false, false);
    }
  }
}

bool exist_wall(int now_y, int now_x, int d) {
  bool flag;
  if      (d == 0) {flag = (hor_wall_me[now_y][now_x] != 0);}
  else if (d == 1) {flag = (ver_wall_me[now_y][now_x] != 0);}
  else if (d == 2) {flag = (hor_wall_me[now_y + 1][now_x] != 0);}
  else if (d == 3) {flag = (ver_wall_me[now_y][now_x + 1] != 0);}
  return flag;
}

bool search_root(int now_y, int now_x) {
  parity_map[now_y][now_x] = 1;
  bool flag = false;
  if (now_y == my_sg[2] && now_x == my_sg[3]) {return true;}
  else {
    if (!exist_wall(now_y, now_x, 0) && parity_map[(now_y + area_num - 1) % area_num][now_x] == 0) {
      flag |= search_root((now_y - 1) % area_num, now_x);
    }
    if (!exist_wall(now_y, now_x, 1) && parity_map[now_y][(now_x + area_num - 1) % area_num] == 0) {
      flag |= search_root(now_y, (now_x - 1) % area_num);
    }
    if (!exist_wall(now_y, now_x, 2) && parity_map[(now_y + 1) % area_num][now_x] == 0) {
      flag |= search_root((now_y + 1) % area_num, now_x);
    }
    if (!exist_wall(now_y, now_x, 3) && parity_map[now_y][(now_x + 1) % area_num] == 0) {
      flag |= search_root(now_y, (now_x + 1) % area_num);
    }
    return flag;
  }
}

bool true_close() {
  for (int h_i = 0; h_i < area_num; h_i++) {
    for (int w_i = 0; w_i < area_num; w_i++) {parity_map[h_i][w_i] = 0;}
  }
  return search_root(my_sg[0], my_sg[1]);
}

void change_wall_val(bool true_me, int index_y, int index_x, int d, int val){
  if (true_me){
    if      (d==0){hor_wall_me[index_y][index_x]   = val;}
    else if (d==1){ver_wall_me[index_y][index_x]   = val;}
    else if (d==2){hor_wall_me[index_y+1][index_x] = val;}
    else if (d==3){ver_wall_me[index_y][index_x+1] = val;}

    if (index_y==0 && d==0){hor_wall_me[area_num][index_x]   = val;}
    if (index_y==area_num-1 && d==2){hor_wall_me[0][index_x] = val;}
    if (index_x==0 && d==1){ver_wall_me[index_y][area_num]   = val;}
    if (index_x==area_num-1 && d==3){ver_wall_me[index_y][0] = val;}
  }else{
    if      (d==0){hor_wall_enemy[index_y][index_x]   = val;}
    else if (d==1){ver_wall_enemy[index_y][index_x]   = val;}
    else if (d==2){hor_wall_enemy[index_y+1][index_x] = val;}
    else if (d==3){ver_wall_enemy[index_y][index_x+1] = val;}
    
    if (index_y==0 && d==0){hor_wall_enemy[area_num][index_x]   = val;}
    if (index_y==area_num-1 && d==2){hor_wall_enemy[0][index_x] = val;}
    if (index_x==0 && d==1){ver_wall_enemy[index_y][area_num]   = val;}
    if (index_x==area_num-1 && d==3){ver_wall_enemy[index_y][0] = val;}
  }
}

void gen_wall() {
  //random_generate_walls
  int num;
  for (int h_i = 0; h_i < area_num; h_i++) {
    for (int w_i = 0; w_i < area_num; w_i++) {
      num = random(1, 15);
      for (int d_i = 3; d_i >=0; d_i--) {
        if (num >= pow(2, d_i)) {
          num -= pow(2, d_i);
          //build_wall
          change_wall_val(true, h_i, w_i, d_i, 1);
        }      
      }
    }
  }

  //stop_dead_in_one's_tracks
  int sum_flag = 0;
  bool flag = false;  
  for (int h_i = 0; h_i < area_num; h_i++) {
    for (int w_i = 0; w_i < area_num; w_i++) {
      sum_flag = ver_wall_me[h_i][w_i + 1] + hor_wall_me[h_i + 1][w_i] + ver_wall_me[h_i][w_i] + hor_wall_me[h_i][w_i];
      if (sum_flag==4){flag = true;}
      if (flag){
        //broke_wall
        change_wall_val(true, h_i, w_i, random(0, 4), 0);
        change_wall_val(true, h_i, w_i, random(0, 4), 0);
        flag = false;
      }
    }
  }
}

void clear_wall(){
  for (int h_i = 0; h_i < area_num; h_i++) {
    for (int w_i = 0; w_i < area_num; w_i++) {
      //broke_wall
      for (int d_i = 3; d_i >=0; d_i--) {change_wall_val(true, h_i, w_i, d_i, 0);}
    }
  }
}

void get_sg(int *sg) {
  if (sg[0]!=-1){  
    draw_area(sg[0], sg[1], true, false);
    draw_area(sg[2], sg[3], true, false);
  }

  int diff_y, diff_x;
  sg[0] = random(0, 5), sg[1] = random(0, 5);
  while (true) {
    sg[2] = random(0, 5), sg[3] = random(0, 5);
    diff_y = abs(sg[0] - sg[2]);
    diff_x = abs(sg[1] - sg[3]);
    if (min(diff_y, 5 - diff_y) + min(diff_x, 5 - diff_x) >= 4) {break;}
  }
  
  draw_area(sg[0], sg[1], true, true);
  draw_triangle(sg[0], sg[1], enemy_pose[2], true);
  draw_area(sg[2], sg[3], true, false);
  draw_char(sg[2], sg[3], true, "G");
}

void correct_wall() {
  while(true){
    get_sg(my_sg);
    gen_wall();
    if (true_close()){break;}
  }  
  set_wall();
  
  while (true) {
    M5.update();
    if (M5.BtnA.wasPressed()) {break;}
    if (M5.Axp.GetBtnPress() == 2) {
      while(true){
        clear_wall();
        get_sg(my_sg);
        gen_wall();
        if (true_close()){break;}
      }
      set_wall();
    }
  }
  draw_marker(0);
}

void draw_marker(int state){
  int left_py=0, left_px=0, radius=int(w1 / 2);
  idx2px(0, 0, true, &left_py, &left_px);
  left_px = int(w1 / 2);
  left_py += int(area_l / 2);
  M5.Lcd.fillCircle(left_px, left_py, radius-2, BLACK);
  if      (state==0){M5.Lcd.drawCircle(left_px, left_py, radius-2, marker_color);}
  else if (state==1){M5.Lcd.fillCircle(left_px, left_py, radius-2, marker_color);}
  else{
    M5.Lcd.drawCircle(left_px, left_py, radius-2, marker_color);
    M5.Lcd.setTextDatum(4);
    if (state==2){
      M5.Lcd.setTextColor(winner_color, BLACK);
      M5.Lcd.drawString("W", left_px, left_py, 1);}
    if (state==3){
      M5.Lcd.setTextColor(loser_color, BLACK);
      M5.Lcd.drawString("L", left_px, left_py, 1);}
    M5.Lcd.setTextDatum(0);
  }
}

void draw_String(String str, int index_y, int index_x, int add_py, int add_px, bool true_me){
  int left_py=0, left_px=0;
  idx2px(index_y, index_x, true_me, &left_py, &left_px);
  left_px = 0;
  M5.Lcd.drawString(str, left_px+add_px, left_py+add_py, 2);
}

void draw_initialize() {
  M5.Lcd.setTextColor(title_str_color, back_color);
  draw_String("E", 0, 0, add_x1, add_y1, false);
  draw_String("N", 1, 0, add_x1, add_y1, false);
  draw_String("E", 2, 0, add_x1, add_y1, false);
  draw_String("M", 3, 0, add_x2, add_y1, false);
  draw_String("Y", 4, 0, add_x1, add_y1, false);
  draw_marker(1);
  draw_String("M", 1, 0, add_x2, add_y1, true);
  draw_String("E", 3, 0, add_x1, add_y1, true);

  change_color(road_l, w1,   h2, road_l,   frame_color);
  change_color(road_l, w - w4, h2, road_l,   frame_color);
  change_color(0, w1,   road_l, h2 + 2 * road_l, frame_color);
  set_area(false);
  change_color(h1 + h2, w1, road_l, h2 + 2 * road_l, frame_color);

  change_color(h1 + h2 + h3 + h4 + road_l, w1,   h2, road_l,   frame_color);
  change_color(h1 + h2 + h3 + h4 + road_l, w - w4, h2, road_l,   frame_color);
  change_color(h1 + h2 + h3 + h4, w1,   road_l, h2 + 2 * road_l, frame_color);
  set_area(true);
  change_color(h1 + h2 + h3 + h4 + h5 + h6, w1, road_l, h2 + 2 * road_l, frame_color);
}

String set_string(int *int_list){
  String send_str = "";
  for (int i=0;i<sizeof(int_list);i++){send_str += String(int_list[i]);}
  send_str += '\r';
  return send_str;
}

void recive_int_data(int *int_list, String rec_d, int len_num){
  String send_str = "";
  for (int i=0;i<len_num;i++){int_list[i] = rec_d.charAt(i) - '0';}
}

void walk(int *pose){
 int now_y = pose[0], now_x = pose[1];
  if      (pose[2] == 0){pose[0] = (now_y + area_num - 1)%area_num;}
  else if (pose[2] == 1){pose[1] = (now_x + area_num - 1)%area_num;}
  else if (pose[2] == 2){pose[0] = (now_y + 1)%area_num;}
  else if (pose[2] == 3){pose[1] = (now_x + 1)%area_num;}
}

void my_tern(){
  draw_marker(1);
  bool my_tern = false;
  int now_d;
  while (true){
    M5.update();

    //change_direction
    if (M5.BtnB.wasPressed()){
      now_d = my_pose[2] + 1;
      my_pose[2] = now_d%4;
      draw_triangle(my_pose[0], my_pose[1], my_pose[2], my_tern);
    }

    //determination_of_direction
    if (M5.BtnA.wasPressed()){
      SerialBT.print(set_string(my_pose));
      
      while(true){
        if (SerialBT.available()){
          recive_int_data(&reaction, SerialBT.readStringUntil('\r'), 1);
          break;
        }
      }

      if (reaction == 0){draw_one_wall(my_pose[0], my_pose[1], my_pose[2], my_tern, true);}
      else{
        draw_area(my_pose[0], my_pose[1], my_tern, true);
        walk(my_pose);
        draw_triangle(my_pose[0], my_pose[1], my_pose[2], my_tern);
        if (reaction == 2){draw_marker(2);}
      }
      break;
    }
  }
}

void enemy_tern(){
  draw_marker(0);
  
  //waiting_enemy
  bool enemy_tern = true;
  while(true){
    if (SerialBT.available()){
      recive_int_data(enemy_pose, SerialBT.readStringUntil('\r'), 3);
      break;
    }
  }

  //Judgment_of_wall_exist
  if (exist_wall(enemy_pose[0], enemy_pose[1], enemy_pose[2])) {
    draw_one_wall(enemy_pose[0], enemy_pose[1], enemy_pose[2], enemy_tern, true);
    draw_triangle(enemy_pose[0], enemy_pose[1], enemy_pose[2], enemy_tern);
    SerialBT.print("0");
  }
  else{
    draw_area(enemy_pose[0], enemy_pose[1], enemy_tern, true);
    walk(enemy_pose);
    draw_triangle(enemy_pose[0], enemy_pose[1], enemy_pose[2], enemy_tern);
    if (enemy_pose[0]==my_sg[2]){
        if (enemy_pose[1]==my_sg[3]){
          draw_marker(3);
          SerialBT.print("2");
        }
        else{SerialBT.print("1");}
    }
    else{SerialBT.print("1");}
  }
}

void setup() {
  M5.begin();
  set_color();
  initialize_wall();
  draw_initialize();
  correct_wall();
  enemy_pose[0] = my_sg[0], enemy_pose[1] = my_sg[1];
  
  SerialBT.begin(my_name, true);
  int connected = 0;
  while (true){
    connected = SerialBT.connect(his_name);
    if (connected != 0){
      SerialBT.print(set_string(my_sg));
      while(true){
        if (SerialBT.available()){
          recive_int_data(enemy_sg, SerialBT.readStringUntil('\r'), 4);
          break;
        }
      }
      break;
    }
  }
  
  draw_char(enemy_sg[2], enemy_sg[3], false, "G");
  my_pose[0] = enemy_sg[0], my_pose[1] = enemy_sg[1];
  draw_triangle(my_pose[0], my_pose[1], my_pose[2], false);
}

void loop() {
  enemy_tern();
  my_tern();
}
