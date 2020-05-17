#include <tuple>
#include <algorithm>
#include <iostream>
#include <vector>

class SplayTree {
public:
    explicit SplayTree(const std::vector<long long> &v) {
        for (int i = 0; i < v.size(); i++) {
            tree = insert(tree, i + 1, v[i]);
        }
    }

    SplayTree(size_t size, long long initialValue) {
        for (int i = 0; i < size; i++) {
            tree = insert(tree, i + 1, initialValue);
        }
    }

    long long operator[](int i) {
        return elementAt(tree, i + 1);
    }

    std::vector<long long> toVector() {
        std::vector<long long> v;
        print(tree, v);
        return v;
    }

    long long getSum(int l, int r) {
        auto res = getSum(tree, l + 1, r + 1);
        tree = res.second;
        return res.first;
    }

    void insert(int i, int x) {
        tree = insert(tree, i + 1, x);
    }

    void remove(int i) {
        tree = remove(tree, i + 1);
    }

    void assign(int l, int r, int x) {
        tree = assign(tree, l + 1, r + 1, x, lastQueryTime);
    }

    void add(int l, int r, int x) {
        tree = add(tree, l + 1, r + 1, x, lastQueryTime);
    }

    void nextPermutation(int l, int r) {
        tree = nextPermutation(tree, l + 1, r + 1);
    }

    void prevPermutation(int l, int r) {
        tree = prevPermutation(tree, l + 1, r + 1);
    }

    size_t size() {
        return getSize(tree);
    }

private:
    struct Node;

    SplayTree::Node *tree = nullptr;

    enum Monotone {
        NON_INCREASING, NON_DECREASING, CONSTANT, NONE
    };

    struct Query {
        static const Query EMPTY;
        int time;
        long long value;

        bool operator==(const Query &q) {
            return time == q.time && value == q.value;
        }

        bool operator!=(const Query &q) {
            return !operator==(q);
        }
    };


    int lastQueryTime = 0;

    struct Node {
        int size = 1;

        long long value;
        long long minValue;
        long long maxValue;

        bool hasRev = false;

        Query addQuery = Query::EMPTY;
        Query assignQuery = Query::EMPTY;

        long long firstValue;
        long long lastValue;

        long long sum;
        Monotone monotone = CONSTANT;

        Node *left = nullptr;
        Node *right = nullptr;
        Node *parent = nullptr;

        explicit Node(long long value) : value(value),
                                         sum(value),
                                         minValue(value),
                                         maxValue(value),
                                         firstValue(value),
                                         lastValue(value) {}


        Node(long long value, Node *left, Node *right) : left(left),
                                                         right(right),
                                                         value(value),
                                                         sum(value),
                                                         minValue(value),
                                                         maxValue(value),
                                                         firstValue(value),
                                                         lastValue(value) {}
    };

    static int getSize(Node *node) {
        return node == nullptr ? 0 : node->size;
    }

    static long long getSum(Node *node) {
        return node == nullptr ? 0 : node->sum;
    }

    static long long getMinValue(Node *node) {
        return node == nullptr ? INT64_MAX : node->minValue;
    }

    static long long getMaxValue(Node *node) {
        return node == nullptr ? INT64_MIN : node->maxValue;
    }


    static void setParent(Node *node, Node *parent) {
        if (node == nullptr) {
            return;
        }
        node->parent = parent;
    }

    static void pushReverse(Node *node) {
        if (node == nullptr || !node->hasRev) {
            return;
        }

        if (node->monotone == NON_DECREASING) {
            node->monotone = NON_INCREASING;
        } else if (node->monotone == NON_INCREASING) {
            node->monotone = NON_DECREASING;
        }

        std::swap(node->lastValue, node->firstValue);
        std::swap(node->left, node->right);

        if (node->left != nullptr) {
            node->left->hasRev ^= true;
        }
        if (node->right != nullptr) {
            node->right->hasRev ^= true;
        }

        node->hasRev = false;
    }


    static void pushAssign(Node *node) {
        if (node == nullptr || node->assignQuery == Query::EMPTY) {
            return;
        }

        if (node->assignQuery.time > node->addQuery.time) {
            node->addQuery = Query::EMPTY;
        } else {
            node->assignQuery.time = node->addQuery.time;
            node->assignQuery.value += node->addQuery.value;

            node->addQuery = Query::EMPTY;
        }

        long long assignValue = node->assignQuery.value;
        node->sum = assignValue * getSize(node);
        node->value = assignValue;
        node->firstValue = assignValue;
        node->lastValue = assignValue;
        node->minValue = assignValue;
        node->maxValue = assignValue;
        node->monotone = CONSTANT;

        if (node->left != nullptr) {
            node->left->assignQuery = node->assignQuery;
        }

        if (node->right != nullptr) {
            node->right->assignQuery = node->assignQuery;
        }

        node->assignQuery = Query::EMPTY;
    }

    static void updateAddQuery(Node *node, Query addQuery) {
        if (node->assignQuery != Query::EMPTY) {
            node->assignQuery.value += addQuery.value;
            node->assignQuery.time = addQuery.time;
        } else {
            node->addQuery.value += addQuery.value;
            node->addQuery.time = addQuery.time;
        }
    }

    static void pushAdd(Node *node) {
        if (node == nullptr || node->addQuery == Query::EMPTY) {
            return;;
        }

        long long addValue = node->addQuery.value;
        node->sum += addValue * getSize(node);
        node->value += addValue;
        node->firstValue += addValue;
        node->lastValue += addValue;
        node->minValue += addValue;
        node->maxValue += addValue;

        if (node->left != nullptr) {
            updateAddQuery(node->left, node->addQuery);
        }

        if (node->right != nullptr) {
            updateAddQuery(node->right, node->addQuery);
        }

        node->addQuery = Query::EMPTY;
    }

    static void push(Node *node) {
        if (node == nullptr) {
            return;
        }

        pushReverse(node);
        pushAssign(node);
        pushAdd(node);
    }

    static bool containsNonIncreasingSequence(Node *node) {
        return node == nullptr ? true : (node->monotone == CONSTANT || node->monotone == NON_INCREASING);
    }

    static bool containsNonDecreasingSequence(Node *node) {
        return node == nullptr ? true : (node->monotone == CONSTANT || node->monotone == NON_DECREASING);
    }

    static bool containsConstantSequence(Node *node) {
        return node == nullptr ? true : node->monotone == CONSTANT;
    }

    static bool containsSequence(Node *node, Monotone type) {
        switch (type) {
            case CONSTANT:
                return containsConstantSequence(node);
            case NON_INCREASING:
                return containsNonIncreasingSequence(node);
            case NON_DECREASING:
                return containsNonDecreasingSequence(node);
            case NONE:
                return true;
        }
        return false;
    }

    static Monotone getMonotone(Node *node) {

        if (containsConstantSequence(node->left) && containsConstantSequence(node->right)) {
            bool isConstant = true;
            if (node->right != nullptr && node->right->minValue != node->value) {
                isConstant = false;
            }
            if (node->left != nullptr && node->left->minValue != node->value) {
                isConstant = false;
            }
            if (isConstant) {
                return CONSTANT;
            }
        }

        if (containsNonDecreasingSequence(node->left) && containsNonDecreasingSequence(node->right)) {
            if (node->right != nullptr && node->right->minValue < node->value) {
                return NONE;
            }
            if (node->left != nullptr && node->left->maxValue > node->value) {
                return NONE;
            }
            return NON_DECREASING;
        }

        if (containsNonIncreasingSequence(node->left) && containsNonIncreasingSequence(node->right)) {
            if (node->right != nullptr && node->right->maxValue > node->value) {
                return NONE;
            }
            if (node->left != nullptr && node->left->minValue < node->value) {
                return NONE;
            }
            return NON_INCREASING;
        }

        return NONE;
    }

    static void update(Node *node) {
        if (node == nullptr) {
            return;
        }

        setParent(node->left, node);
        setParent(node->right, node);

        push(node->left);
        push(node->right);

        node->sum = getSum(node->left) + getSum(node->right) + node->value;
        node->size = getSize(node->left) + getSize(node->right) + 1;
        node->minValue = std::min(std::min(getMinValue(node->left), getMinValue(node->right)), node->value);
        node->maxValue = std::max(std::max(getMaxValue(node->left), getMaxValue(node->right)), node->value);
        node->monotone = getMonotone(node);

        if (node->left != nullptr) {
            node->firstValue = node->left->firstValue;
        } else {
            node->firstValue = node->value;
        }

        if (node->right != nullptr) {
            node->lastValue = node->right->lastValue;
        } else {
            node->lastValue = node->value;
        }
    }

    static void rotate(Node *parent, Node *child) {
        Node *grandParent = parent->parent;
        if (grandParent != nullptr) {
            if (grandParent->left == parent) {
                grandParent->left = child;
            } else {
                grandParent->right = child;
            }
        }

        if (parent->left == child) {
            parent->left = child->right;
            child->right = parent;
        } else {
            parent->right = child->left;
            child->left = parent;
        }

        update(child);
        update(parent);
        update(grandParent);

        setParent(child, grandParent);
    }

    static Node *splay(Node *v) {
        push(v);
        if (v->parent == nullptr) {
            update(v);
            return v;
        }

        Node *parent = v->parent;
        Node *grandParent = parent->parent;

        if (grandParent == nullptr) {
            rotate(parent, v);
            update(v);
            return v;
        }
        bool zigZig = (grandParent->left == parent) == (parent->left == v);
        if (zigZig) {
            rotate(grandParent, parent);
            rotate(parent, v);
        } else {
            rotate(parent, v);
            rotate(grandParent, v);
        }
        update(grandParent);
        update(parent);
        return splay(v);
    }

    static Node *find(Node *v, int i) {
        push(v);
        if (v == nullptr) {
            return nullptr;
        }
        int currentSize = getSize(v->left) + 1;

        if (i == currentSize) {
            return v;
        }
        if (i < currentSize && v->left != nullptr) {
            return find(v->left, i);
        }
        if (i > currentSize && v->right != nullptr) {
            return find(v->right, i - currentSize);
        }
        return v;
    }

    static long long elementAt(Node *node, int i) {
        return find(node, i)->value;
    }

    static std::pair<Node *, Node *> split(Node *root, int i) {
        if (root == nullptr) {
            return {nullptr, nullptr};
        }
        push(root);
        root = splay(find(root, i));

        if (getSize(root) < i) {
            Node *right = root->right;
            setParent(right, nullptr);

            root->right = nullptr;
            update(root);
            update(right);

            return {root, right};
        } else {
            Node *left = root->left;
            setParent(left, nullptr);

            root->left = nullptr;
            update(root);
            update(left);

            return {left, root};
        }
    }

    static Node *merge(Node *left, Node *right) {
        push(left);
        push(right);

        if (right == nullptr) {
            return left;
        }
        if (left == nullptr) {
            return right;
        }

        left = splay(find(left, getSize(left)));

        left->right = right;


        update(right);
        update(left);

        return left;
    }

    static Node *insert(Node *root, int pos, long long value) {
        if (root == nullptr) {
            return new Node(value);
        }

        auto splitted = split(root, pos);

        Node *left = splitted.first;
        Node *right = splitted.second;
        root = new Node(value, left, right);
        update(root);
        return root;
    }

    static Node *remove(Node *root, int i) {
        root = splay(find(root, i));

        setParent(root->left, nullptr);
        setParent(root->right, nullptr);

        return merge(root->left, root->right);
    }

    static void print(Node *root, std::vector<long long> &accumulator) {
        push(root);
        if (root == nullptr) {
            return;
        }
        print(root->left, accumulator);
        accumulator.push_back(root->value);
        print(root->right, accumulator);
    }

    static std::tuple<Node *, Node *, Node *> extractSegment(Node *root, int l, int r) {
        Node *t1;
        Node *t2;
        Node *t3;

        auto spl = split(root, l);
        t1 = spl.first;
        t2 = spl.second;

        spl = split(t2, r - l + 2);
        t2 = spl.first;
        t3 = spl.second;

        return std::make_tuple(t1, t2, t3);
    }

    static Node *add(Node *root, int l, int r, long long x, int &lastQueryTime) {
        auto splitted = extractSegment(root, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        push(t2);
        t2->addQuery = {++lastQueryTime, x};

        return merge(merge(t1, t2), t3);
    }

    static Node *assign(Node *root, int l, int r, long long x, int &lastQueryTime) {
        auto splitted = extractSegment(root, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        push(t2);
        t2->assignQuery = {++lastQueryTime, x};

        return merge(merge(t1, t2), t3);
    }

    static Node *reverse(Node *root, int l, int r) {
        auto splitted = extractSegment(root, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        push(t2);
        t2->hasRev ^= true;

        return merge(merge(t1, t2), t3);
    }

    static std::pair<long long, Node *> getSum(Node *node, int l, int r) {
        auto splitted = extractSegment(node, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        long long sum = getSum(t2);

        node = merge(merge(t1, t2), t3);
        return {sum, node};
    }


    static std::pair<long long, Node *> getMin(Node *root, int l, int r) {
        auto splitted = extractSegment(root, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        long long minValue = getMinValue(t2);

        root = merge(merge(t1, t2), t3);
        return {minValue, root};
    }

    static int indexOf(Node *root, Node *v) {
        std::vector<Node *> path = {v};
        while (v->parent != nullptr) {
            path.push_back(v->parent);
            v = v->parent;
        }
        std::reverse(path.begin(), path.end());
        int pos = getSize(root->left) + 1;
        for (int i = 0; i + 1 < path.size(); i++) {
            Node *currentVertex = path[i];
            Node *child = path[i + 1];
            push(currentVertex);
            if (child == currentVertex->left) {
                pos -= getSize(currentVertex->left->right) + 1;
            } else {
                pos += getSize(currentVertex->right->left) + 1;
            }
        }
        return pos;
    }


    static int getMonotoneSuffix(Node *v, Monotone type) {
        push(v);
        update(v);

        if (v == nullptr) {
            return 0;
        }
        if (containsSequence(v, type)) {
            return getSize(v);
        }

        int ans = getMonotoneSuffix(v->right, type);

        if (getSize(v->right) == ans) {
            if (type == NON_INCREASING && (v->right ? v->value >= v->right->firstValue : true)) {
                ans++;
                update(v);
                if ((v->left ? v->left->lastValue >= v->value : true)) {
                    ans += getMonotoneSuffix(v->left, type);
                }
            } else if (type == NON_DECREASING && (v->right ? v->value <= v->right->firstValue : true)) {
                ans++;
                update(v);
                if ((v->left ? v->left->lastValue <= v->value : true)) {
                    ans += getMonotoneSuffix(v->left, type);
                }
            }
        }
        return std::max(ans, 1);
    }

    static Node *swapSegments(Node *root, int l1, int r1, int l2, int r2) {
        auto splitted1 = extractSegment(root, l1 + 1, r1 + 1);

        Node *t1 = std::get<0>(splitted1);
        Node *t2 = std::get<1>(splitted1);
        Node *t3 = std::get<2>(splitted1);

        l2 -= r1;
        r2 -= r1;
        auto splitted2 = extractSegment(t3, l2, r2);
        Node *t4 = std::get<0>(splitted2);
        Node *t5 = std::get<1>(splitted2);
        Node *t6 = std::get<2>(splitted2);

        return merge(merge(merge(merge(t1, t5), t4), t2), t6);
    }

    static Node *getMinimalGreater(Node *node, long long value) {
        if (node == nullptr) {
            return nullptr;
        }

        push(node);

        if (node->value > value) {
            Node *rightAns = getMinimalGreater(node->right, value);
            return rightAns != nullptr ? rightAns : node;
        }

        return getMinimalGreater(node->left, value);
    }

    static std::pair<Node *, Node *> getMinimalGreater(Node *node, int l, int r, long long value) {
        auto splitted = extractSegment(node, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        return {getMinimalGreater(t2, value), merge(merge(t1, t2), t3)};
    }

    static Node *getMaximalLess(Node *node, long long value) {
        if (node == nullptr) {
            return nullptr;
        }

        push(node);

        if (node->value < value) {
            Node *rightAns = getMaximalLess(node->right, value);
            return rightAns != nullptr ? rightAns : node;
        }
        return getMaximalLess(node->left, value);
    }


    static std::pair<Node *, Node *> getMaximalLess(Node *node, int l, int r, long long value) {
        auto splitted = extractSegment(node, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        return {getMaximalLess(t2, value), merge(merge(t1, t2), t3)};
    }

    static Node *nextPermutation(Node *root, int l, int r) {
        auto splitted = extractSegment(root, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        Node *v = t2;
        int nonIncreasingSuffixLength = getMonotoneSuffix(v, NON_INCREASING);
        int pivotPosition = std::max(1, v->size - nonIncreasingSuffixLength);
        long long pivotValue = elementAt(v, pivotPosition);

        auto res = getMinimalGreater(v, pivotPosition + 1, getSize(v), pivotValue);
        Node *firstGreater = res.first;
        v = res.second;

        if (firstGreater == nullptr) {
            v = reverse(v, 1, getSize(v));
            return merge(merge(t1, v), t3);
        }

        int indexOfFirstGreater = indexOf(v, firstGreater);

        v = swapSegments(v, pivotPosition - 1, pivotPosition - 1, indexOfFirstGreater - 1, indexOfFirstGreater - 1);

        v = reverse(v, pivotPosition + 1, getSize(v));

        return merge(merge(t1, v), t3);
    }

    static Node *prevPermutation(Node *root, int l, int r) {
        auto splitted = extractSegment(root, l, r);

        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        Node *v = t2;
        int nonDecreasingSuffixLength = getMonotoneSuffix(v, NON_DECREASING);
        int pivotPosition = std::max(1, v->size - nonDecreasingSuffixLength);
        long long pivotValue = elementAt(v, pivotPosition);

        auto res = getMaximalLess(v, pivotPosition + 1, getSize(v), pivotValue);
        Node *firstLess = res.first;
        v = res.second;

        if (firstLess == nullptr) {
            v = reverse(v, 1, getSize(v));
            return merge(merge(t1, v), t3);
        }

        int indexOfFirstLess = indexOf(v, firstLess);

        v = swapSegments(v, pivotPosition - 1, pivotPosition - 1, indexOfFirstLess - 1, indexOfFirstLess - 1);

        v = reverse(v, pivotPosition + 1, getSize(v));

        return merge(merge(t1, v), t3);
    }
};

const SplayTree::Query SplayTree::Query::EMPTY = {0, 0};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);

    int n;
    std::cin >> n;
    std::vector<long long> initialData(n);
    for (int i = 0; i < n; i++) {
        std::cin >> initialData[i];
    }
    SplayTree tree(initialData);

    int q;
    std::cin >> q;
    for (int i = 0; i < q; i++) {
        int type;
        std::cin >> type;
        if (type == 3) {
            int pos;
            std::cin >> pos;
            tree.remove(pos);
        }
        if (type == 2) {
            long long pos, x;
            std::cin >> x >> pos;
            tree.insert(pos, x);
        }

        if (type == 1) {
            int l, r;
            std::cin >> l >> r;
            std::cout << tree.getSum(l, r) << "\n";
        }
        if (type == 4) {
            long long x, l, r;
            std::cin >> x >> l >> r;
            tree.assign(l, r, x);
        }

        if (type == 5) {
            long long x, l, r;
            std::cin >> x >> l >> r;
            tree.add(l, r, x);
        }

        if (type == 6) {
            int l, r;
            std::cin >> l >> r;
            tree.nextPermutation(l, r);
        }

        if (type == 7) {
            int l, r;
            std::cin >> l >> r;
            tree.prevPermutation(l, r);
        }
    }

    auto treeVector = tree.toVector();
    for (auto e : treeVector) {
        std::cout << e << " ";
    }
}
