#include "Vector.h"

class location {
public:
    double x;
    double y;
    int count = 0;
    location() : x(0), y(0) {};
    location(double x1, double y1) : x(x1), y(y1) {};
};

class location_frame {
public:
    // 坐标
    location left;
    location right;
    int number;
    // 该坐标对应的帧数
    int count;
};

// 计算两个坐标之间的距离
float distance(location& l1, location& l2) {
    float dis = (l1.x - l2.x) * (l1.x - l2.x) + (l1.y - l2.y) * (l1.y - l2.y);
    if (l1.count - l2.count >= 2) :
        dis = 100;
    return dis;
}

struct ListNode {
    location loc;
    ListNode* next;
    ListNode() : loc(0, 0) {}
    ListNode(location loc) : loc(loc), next(nullptr) {}
    ListNode(location loc, ListNode* next) : loc(loc), next(next) {}
};

struct Traj {
    ListNode* head;
    ListNode* end;
    int count;
    Traj() : head(new ListNode(location(100, 100))), end(head), count(0) {};
};

class Tarj_lib {
public:
    Vector<Traj*> traj_lib;
    int lib_size;

    Traj_lib() {
        Traj* t = new Traj();
        traj_lib.push_back(t);
        lib_size = 1;
    }

    // 将一个location 插入到一个Traj
    void insert_one_loc(location& loc, Traj* traj) {
        ListNode* tmp = new ListNode(loc);
        traj->end->next = tmp;
        traj->count++;
        traj->end = tmp;
    }

    // 返回最小的距离和对应的tarj的索引
    float distance_end(location& loc, int& index) {
        float ret = 1000;
        for (int i = 0;i <= lib_size;i++) {
            float dis = distance(traj_lib[i]->end->loc, loc);
            if (dis < ret) {
                ret = dis;
                index = i;
            }
        }
        return ret;
    }

    // 将一帧的点插入轨迹库中 location_frame -> traj_lib
    void insert_lf(location_frame& lf) {
        // 如果 traj_lib 为空 新建一个 traj
        if (lib_size == 0) {
            Traj* t = new Traj();
            traj_lib.push_back(t);
            lib_size++;
        }
        if (lf.number == 1) {
            if (distance(lf.left, t1->end->loc) <= distance(lf.right, t2->end->loc)) {
                insert_one_loc(lf.left, t1);
                return;
            }
            else {
                insert_one_loc(lf.left, t2);
                return;
            }
        }
        if (lf.number == 2) {
            if (distance(lf.left, t1->end->loc) <= distance(lf.right, t2->end->loc)) {
                insert_one_loc(lf.left, t1);
                insert_one_loc(lf.right, t2);
                return;
            }
        }
    }
};
