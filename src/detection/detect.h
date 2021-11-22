#include "Vector.h"
#include "Arduino.h"

class location
{
public:
    double x;
    double y;
    int count = 0;
    location() : x(0), y(0){};
    location(double x1, double y1) : x(x1), y(y1){};
};

class location_frame
{
public:
    // 坐标
    location left;
    location right;
    int number;
    // 该坐标对应的帧数
    int count;
};

// 计算两个坐标之间的距离
float distance(location &l1, location &l2)
{
    float dis = (l1.x - l2.x) * (l1.x - l2.x) + (l1.y - l2.y) * (l1.y - l2.y);
    if (l1.count - l2.count >= 3)
        dis = 100;
    return dis;
}

struct ListNode
{
    location loc;
    ListNode *next;
    ListNode() : loc(0, 0) {}
    ListNode(location loc) : loc(loc), next(nullptr) {}
    ListNode(location loc, ListNode *next) : loc(loc), next(next) {}
};

struct Traj
{
    ListNode *head;
    ListNode *end;
    int count;
    Traj() : head(new ListNode(location(100, 100))), end(head), count(0){};
};

class Traj_lib
{
public:
    Vector<Traj *> traj_lib;
    int end_index;

    Traj_lib()
    {
        Traj *t1 = new Traj();
        // Traj *t2 = new Traj();
        traj_lib.push_back(t1);
        // traj_lib.push_back(t2);
        end_index = 0;
    }

    // 将一个location 插入到一个Traj
    void insert_one_loc(location &loc, Traj *traj)
    {
        ListNode *tmp = new ListNode(loc);
        traj->end->next = tmp;
        traj->count++;
        traj->end = tmp;
    }

    // 返回最小的距离和对应的tarj的索引
    float distance_to_ends(location &loc, int &index)
    {
        float ret = 1000;
        if (end_index == 0)
        {
            float dis = distance(traj_lib[0]->end->loc, loc);
            index = 0;
            return dis;
        }
        for (int i = 0; i <= end_index; i++)
        {
            float dis = distance(traj_lib[i]->end->loc, loc);
            if (dis < ret)
            {
                ret = dis;
                index = i;
            }
        }
        return ret;
    }

    // 添加一个 loc 到 traj_lib[index]
    void insert_to_end(location &loc, int index)
    {
        ListNode *node = new ListNode(loc);
        traj_lib[index]->end->next = node;
        traj_lib[index]->end = node;
    }

    void traj_lib_build(location_frame &lf)
    {
        // 如果 traj_lib 为空 新建一个 traj
        if (lf.number == 1)
        {
            int index;
            float min_dis = distance_to_ends(lf.left, index);
            if (min_dis < 100)
            {
                insert_to_end(lf.left, index);
            }
            else
            {
                Traj *t = new Traj();
                traj_lib.push_back(t);
                end_index += 1;
                insert_to_end(lf.left, end_index);
            }
        }
        if (lf.number == 2)
        {
            int index;
            float min_dis = distance_to_ends(lf.left, index);
            if (min_dis < 100)
            {
                insert_to_end(lf.left, index);
            }
            else
            {
                Traj *t = new Traj();
                traj_lib.push_back(t);
                end_index += 1;
                insert_to_end(lf.left, end_index);
            }
            min_dis = distance_to_ends(lf.right, index);
            if (min_dis < 100)
            {
                insert_to_end(lf.right, index);
            }
            else
            {
                Traj *t = new Traj();
                traj_lib.push_back(t);
                end_index += 1;
                insert_to_end(lf.right, end_index);
            }
        }
    }

    String print_traj()
    {
        String a = "";
        for (int i = 0; i <= end_index; i++)
        {
            a += "traj: ";
            a += i;
            ListNode *node = traj_lib[i]->head;
            while (node)
            {
                a += "(";
                a += node->loc.x;
                a += ",";
                a += node->loc.y;
                a += ")";
                a += "->";
                node = node->next;
            }
            a += "end";
        }
        return a;
    }
};
