//
// Created by Александр on 05.11.2019.
//

typedef int elem_t;

const int MAX_size_ = 64;

struct List {

    int size_;

    elem_t* data_;
    int*    next_;
    int*    prev_;

    int head_;
    int tail_;
    int free_;

} list;

void list_inti() {

    list.size_ = 0;
    list.head_ = 0;
    list.tail_ = 0;
    list.free_ = 1;

    list.data_ = (elem_t*)calloc(MAX_size_ + 1, sizeof(elem_t));
    list.next_ =    (int*)calloc(MAX_size_ + 1, sizeof(int));
    list.prev_ =    (int*)calloc(MAX_size_ + 1, sizeof(int));

    // filling array for free pointer

    list.free_ = 1;
    for (int i = 1; i < MAX_size_; i++)
        list.next_[i] = i + 1;
    list.next_[MAX_size_] = 0;
    
}


void list_deinit() {
    free(list.data_);
    free(list.next_);
    free(list.prev_);
}

int add_first_elem(elem_t value) {

    if (list.free_ == 0) {
        return -1;
    }

    if (list.size_ != 0) {
        return -1;
    }

    int new_pos = list.free_;
    list.free_ = list.next_[list.free_];

    list.head_ = list.tail_ = new_pos;

    list.next_[new_pos] = list.prev_[new_pos] = 0;

    list.data_[new_pos] = value;

    list.size_++;

    return new_pos;
}

/*!
 * Pushs the element at the end of List
 *
 * @param[in] value value for adding
 * @return phisical number of added element
 */

int push_back(elem_t value) {
     

    if (list.free_ == 0) {
        return -1;
    }

    if (list.size_ == 0) {
        return add_first_elem(value);
    }

    int new_pos = list.free_;
    list.free_ = list.next_[list.free_];

    list.next_[list.tail_] = new_pos;
    list.prev_[new_pos] = list.tail_;
    list.next_[new_pos] = 0;
    list.tail_ = new_pos;

    list.data_[new_pos] = value;

    list.size_++;

    return new_pos;

}

int push_front(elem_t value) {

    if (list.free_ == 0) {
        return -1;
    }

    if (list.size_ == 0) {
        return add_first_elem(value);
    }

    int new_pos = list.free_;
    list.free_ = list.next_[list.free_];

    list.prev_[list.head_] = new_pos;
    list.next_[new_pos] = list.head_;
    list.prev_[new_pos] = 0;
    list.head_ = new_pos;

    list.data_[new_pos] = value;

    list.size_++;

    return new_pos;
}

int insert_after(int pos, elem_t value) {

    if (pos <= 0 || pos > MAX_size_) {
        return -1;
    }

    if (list.free_ == 0) {
        return -1;
    }

    if (pos == list.tail_) {
        return push_back(value);
    }

    int new_pos = list.free_;
    list.free_ = list.next_[list.free_];

    list.prev_[list.next_[pos]] = new_pos;
    list.prev_[new_pos] = pos;
    list.next_[new_pos] = list.next_[pos];
    list.next_[pos] = new_pos;

    list.data_[new_pos] = value;

    list.size_++;

    return new_pos;
}

// int insert_before(int pos, elem_t value) {
     
//     if (pos <= 0 || pos > MAX_size_) {
//         return -1;
//     }

//     if (list.free_ == 0) {
//         return -1;
//     }

//     if (pos == list.head_) {
//         return push_front(value);
//     }

//     return insert_after(get_list.prev_index(pos), value);
// }

int pop_back() {
    
    int del_pos = list.tail_;
    if (del_pos == 0) {
        return -1;
    }

    list.tail_ = list.prev_[del_pos];
    list.next_[list.prev_[del_pos]] = 0; // hide if you want to show all

    list.next_[del_pos] = list.free_;
    list.free_ = del_pos;

    list.size_--;

    if (list.size_ == 0)
        list.head_ = 0;

    return 0;
}

int pop_front() {
     
    int del_pos = list.head_;
    if (del_pos == 0) {
        return -1;
    }

    list.head_ = list.next_[del_pos];
    list.prev_[list.next_[del_pos]] = 0;

    list.next_[del_pos] = list.free_;
    list.free_ = del_pos;

    list.size_--;

    if (list.size_ == 0)
        list.tail_ = 0;

    return 0;
}

int erase(int del_pos) {
     
    if (del_pos <= 0 ||del_pos > MAX_size_) {
        return -1;
    }

    if (del_pos == list.head_)
        return pop_front();

    if (del_pos == list.tail_)
        return pop_back();

    list.next_[list.prev_[del_pos]] = list.next_[del_pos];
    list.prev_[list.next_[del_pos]] = list.prev_[del_pos];

    list.next_[del_pos] = list.free_;
    list.free_ = del_pos;

    list.size_--;
     
    return 0;
}

int find_index_by_value(int value) {

    for (int i = list.head_; i != 0; i = list.next_[i])
        if (list.data_[i] == value)
            return i;

    return 0;
}
