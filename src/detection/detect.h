#include "Vector.h"

class location {
public:
    double x;
    double y;
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
    Traj* t1 = new Traj();
    Traj* t2 = new Traj();

    void empty_lib()     {
        t1 = new Traj();
        t2 = new Traj();
    }
    void insert_one_loc(location& loc, Traj* traj)     {
        ListNode* tmp = new ListNode(loc);
        traj->end->next = tmp;
        traj->count++;
        traj->end = tmp;
    }

    void insert_lf(location_frame& lf)     {
        if (lf.number == 1)         {
            if (distance(lf.left, t1->end->loc) <= distance(lf.right, t2->end->loc))             {
                insert_one_loc(lf.left, t1);
                return;
            }
            else             {
                insert_one_loc(lf.left, t2);
                return;
            }
        }
        if (lf.number == 2)         {
            if (distance(lf.left, t1->end->loc) <= distance(lf.right, t2->end->loc))             {
                insert_one_loc(lf.left, t1);
                insert_one_loc(lf.right, t2);
                return;
            }
        }
    }
};
