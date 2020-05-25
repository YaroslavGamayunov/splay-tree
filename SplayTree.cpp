#include <tuple>
#include <algorithm>
#include <iostream>
#include <vector>
#include <functional>

class SplayTree {
public:
    SplayTree() = default;

    explicit SplayTree(const std::vector<long long> &v) {
        for (int i = 0; i < v.size(); i++) {
            tree_ = insert_(tree_, i + 1, v[i]);
        }
    }

    SplayTree(size_t size, long long initialValue) {
        for (int i = 0; i < size; i++) {
            tree_ = insert_(tree_, i + 1, initialValue);
        }
    }

    SplayTree(const SplayTree &other) {
        tree_ = other.tree_ == nullptr ? new Node(*other.tree_) : nullptr;
        lastQueryTime_ = other.lastQueryTime_;
    }

    SplayTree &operator=(const SplayTree &other) {
        // copy-and-swap idiom
        if (this == &other) {
            return *this;
        }
        SplayTree tmp(other);
        std::swap(tree_, tmp.tree_);
        std::swap(lastQueryTime_, tmp.lastQueryTime_);
        return *this;
    }

    ~SplayTree() {
        delete tree_;
    }

    long long operator[](int i) {
        return elementAt_(tree_, i + 1);
    }

    long long getSum(int l, int r) {
        auto res = getSum_(tree_, l + 1, r + 1);
        tree_ = res.second;
        return res.first;
    }

    void insert(int i, int x) {
        tree_ = insert_(tree_, i + 1, x);
    }

    void remove(int i) {
        tree_ = remove_(tree_, i + 1);
    }

    void assign(int l, int r, int x) {
        tree_ = assign_(tree_, l + 1, r + 1, x, lastQueryTime_);
    }

    void add(int l, int r, int x) {
        tree_ = add_(tree_, l + 1, r + 1, x, lastQueryTime_);
    }

    void nextPermutation(int l, int r) {
        tree_ = nextPermutation_(tree_, l + 1, r + 1);
    }

    void prevPermutation(int l, int r) {
        tree_ = prevPermutation_(tree_, l + 1, r + 1);
    }

    size_t size() const {
        return getSize_(tree_);
    }

    std::vector<long long> toVector() {
        std::vector<long long> result;
        traverse_(tree_, [&result](Node *node) {
            result.push_back(node->value);
        });
        return result;
    }

private:
    struct Node;
    SplayTree::Node *tree_ = nullptr;

    struct Query;
    int lastQueryTime_ = 0;

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

        Node(const Node &other) {
            size = other.size;
            value = other.value;
            minValue = other.minValue;
            maxValue = other.maxValue;
            hasRev = other.hasRev;
            addQuery = other.addQuery;
            assignQuery = other.assignQuery;
            firstValue = other.firstValue;
            lastValue = other.lastValue;
            sum = other.sum;
            monotone = other.monotone;

            if (other.left != nullptr) {
                left = new Node(*other.left);
            }
            if (other.right != nullptr) {
                right = new Node(*other.right);
            }
        }

        Node &operator=(const Node &other) {
            // copy-and-swap idiom
            if (this == &other) {
                return *this;
            }
            Node tmp(other);
            swap(*this, tmp);
            return *this;
        }

        ~Node() {
            delete left;
            delete right;
        }

        friend void swap(Node &a, Node &b) {
            std::swap(a.size, b.size);
            std::swap(a.value, b.value);
            std::swap(a.minValue, b.minValue);
            std::swap(a.maxValue, b.maxValue);
            std::swap(a.hasRev, b.hasRev);
            std::swap(a.addQuery, b.addQuery);
            std::swap(a.assignQuery, b.assignQuery);
            std::swap(a.firstValue, b.firstValue);
            std::swap(a.lastValue, b.lastValue);
            std::swap(a.sum, b.sum);
            std::swap(a.monotone, b.monotone);
            std::swap(a.left, b.left);
            std::swap(a.right, b.right);
            std::swap(a.parent, b.parent);
        }
    };

    static int getSize_(Node *node) {
        return node == nullptr ? 0 : node->size;
    }

    static long long getSum_(Node *node) {
        return node == nullptr ? 0 : node->sum;
    }

    static long long getMinValue_(Node *node) {
        return node == nullptr ? INT64_MAX : node->minValue;
    }

    static long long getMaxValue_(Node *node) {
        return node == nullptr ? INT64_MIN : node->maxValue;
    }


    static void setParent_(Node *node, Node *parent) {
        if (node == nullptr) {
            return;
        }
        node->parent = parent;
    }

    static void pushReverse_(Node *node) {
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


    static void pushAssign_(Node *node) {
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
        node->sum = assignValue * getSize_(node);
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

    static void updateAddQuery_(Node *node, Query addQuery) {
        if (node->assignQuery != Query::EMPTY) {
            node->assignQuery.value += addQuery.value;
            node->assignQuery.time = addQuery.time;
        } else {
            node->addQuery.value += addQuery.value;
            node->addQuery.time = addQuery.time;
        }
    }

    static void pushAdd_(Node *node) {
        if (node == nullptr || node->addQuery == Query::EMPTY) {
            return;
        }

        long long addValue = node->addQuery.value;
        node->sum += addValue * getSize_(node);
        node->value += addValue;
        node->firstValue += addValue;
        node->lastValue += addValue;
        node->minValue += addValue;
        node->maxValue += addValue;

        if (node->left != nullptr) {
            updateAddQuery_(node->left, node->addQuery);
        }

        if (node->right != nullptr) {
            updateAddQuery_(node->right, node->addQuery);
        }

        node->addQuery = Query::EMPTY;
    }

    static void push_(Node *node) {
        if (node == nullptr) {
            return;
        }

        pushReverse_(node);
        pushAssign_(node);
        pushAdd_(node);
    }

    static bool containsNonIncreasingSequence_(Node *node) {
        return node == nullptr ? true : (node->monotone == CONSTANT || node->monotone == NON_INCREASING);
    }

    static bool containsNonDecreasingSequence_(Node *node) {
        return node == nullptr ? true : (node->monotone == CONSTANT || node->monotone == NON_DECREASING);
    }

    static bool containsConstantSequence_(Node *node) {
        return node == nullptr ? true : node->monotone == CONSTANT;
    }

    static bool containsSequence_(Node *node, Monotone type) {
        switch (type) {
            case CONSTANT:
                return containsConstantSequence_(node);
            case NON_INCREASING:
                return containsNonIncreasingSequence_(node);
            case NON_DECREASING:
                return containsNonDecreasingSequence_(node);
            case NONE:
                return true;
        }
        return false;
    }

    static Monotone getMonotone_(Node *node) {

        if (containsConstantSequence_(node->left) && containsConstantSequence_(node->right)) {
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

        if (containsNonDecreasingSequence_(node->left) && containsNonDecreasingSequence_(node->right)) {
            if (node->right != nullptr && node->right->minValue < node->value) {
                return NONE;
            }
            if (node->left != nullptr && node->left->maxValue > node->value) {
                return NONE;
            }
            return NON_DECREASING;
        }

        if (containsNonIncreasingSequence_(node->left) && containsNonIncreasingSequence_(node->right)) {
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

    static void update_(Node *node) {
        if (node == nullptr) {
            return;
        }

        setParent_(node->left, node);
        setParent_(node->right, node);

        push_(node->left);
        push_(node->right);

        node->sum = getSum_(node->left) + getSum_(node->right) + node->value;
        node->size = getSize_(node->left) + getSize_(node->right) + 1;
        node->minValue = std::min(std::min(getMinValue_(node->left), getMinValue_(node->right)), node->value);
        node->maxValue = std::max(std::max(getMaxValue_(node->left), getMaxValue_(node->right)), node->value);
        node->monotone = getMonotone_(node);

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

    static void rotate_(Node *parent, Node *child) {
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

        update_(child);
        update_(parent);
        update_(grandParent);

        setParent_(child, grandParent);
    }

    static Node *splay_(Node *v) {
        push_(v);
        if (v->parent == nullptr) {
            update_(v);
            return v;
        }

        Node *parent = v->parent;
        Node *grandParent = parent->parent;

        if (grandParent == nullptr) {
            rotate_(parent, v);
            update_(v);
            return v;
        }
        bool zigZig = (grandParent->left == parent) == (parent->left == v);
        if (zigZig) {
            rotate_(grandParent, parent);
            rotate_(parent, v);
        } else {
            rotate_(parent, v);
            rotate_(grandParent, v);
        }
        update_(grandParent);
        update_(parent);
        return splay_(v);
    }

    static Node *find_(Node *v, int i) {
        push_(v);
        if (v == nullptr) {
            return nullptr;
        }
        int currentSize = getSize_(v->left) + 1;

        if (i == currentSize) {
            return v;
        }
        if (i < currentSize && v->left != nullptr) {
            return find_(v->left, i);
        }
        if (i > currentSize && v->right != nullptr) {
            return find_(v->right, i - currentSize);
        }
        return v;
    }

    static long long elementAt_(Node *node, int i) {
        return find_(node, i)->value;
    }

    static std::pair<Node *, Node *> split_(Node *root, int i) {
        if (root == nullptr) {
            return {nullptr, nullptr};
        }
        push_(root);
        root = splay_(find_(root, i));

        if (getSize_(root) < i) {
            Node *right = root->right;
            setParent_(right, nullptr);

            root->right = nullptr;
            update_(root);
            update_(right);

            return {root, right};
        } else {
            Node *left = root->left;
            setParent_(left, nullptr);

            root->left = nullptr;
            update_(root);
            update_(left);

            return {left, root};
        }
    }

    static Node *merge_(Node *left, Node *right) {
        push_(left);
        push_(right);

        if (right == nullptr) {
            return left;
        }
        if (left == nullptr) {
            return right;
        }

        left = splay_(find_(left, getSize_(left)));

        left->right = right;


        update_(right);
        update_(left);

        return left;
    }

    static void traverse_(Node *root, const std::function<void(Node *)> &operation) {
        push_(root);
        if (root == nullptr) {
            return;
        }
        traverse_(root->left, operation);
        operation(root);
        traverse_(root->right, operation);
    }

    static std::tuple<Node *, Node *, Node *> extractSegment_(Node *root, int l, int r) {
        Node *t1;
        Node *t2;
        Node *t3;

        auto spl = split_(root, l);
        t1 = spl.first;
        t2 = spl.second;

        spl = split_(t2, r - l + 2);
        t2 = spl.first;
        t3 = spl.second;

        return std::make_tuple(t1, t2, t3);
    }

    static Node *makeOperationOnSubSegment_(Node *root, int l, int r,
                                            const std::function<Node *(Node *)> &operation) {
        auto splitted = extractSegment_(root, l, r);
        Node *t1 = std::get<0>(splitted);
        Node *t2 = std::get<1>(splitted);
        Node *t3 = std::get<2>(splitted);

        t2 = operation(t2);
        return merge_(merge_(t1, t2), t3);
    }

    static Node *insert_(Node *root, int pos, long long value) {
        if (root == nullptr) {
            return new Node(value);
        }

        auto splitted = split_(root, pos);

        Node *left = splitted.first;
        Node *right = splitted.second;
        root = new Node(value, left, right);
        update_(root);
        return root;
    }

    static Node *remove_(Node *node, int i) {
        return makeOperationOnSubSegment_(node, i, i, [](Node *treeSegment) {
            delete treeSegment;
            return nullptr;
        });
    }


    static Node *add_(Node *node, int l, int r, long long x, int &lastQueryTime) {
        return makeOperationOnSubSegment_(node, l, r, [&lastQueryTime, &x](Node *treeSegment) {
            push_(treeSegment);
            treeSegment->addQuery = {++lastQueryTime, x};
            return treeSegment;
        });
    }

    static Node *assign_(Node *node, int l, int r, long long x, int &lastQueryTime) {
        return makeOperationOnSubSegment_(node, l, r, [&lastQueryTime, &x](Node *treeSegment) {
            push_(treeSegment);
            treeSegment->assignQuery = {++lastQueryTime, x};
            return treeSegment;
        });
    }

    static Node *reverse_(Node *node, int l, int r) {
        return makeOperationOnSubSegment_(node, l, r, [](Node *treeSegment) {
            push_(treeSegment);
            treeSegment->hasRev ^= true;
            return treeSegment;
        });
    }

    static std::pair<long long, Node *> getSum_(Node *node, int l, int r) {
        long long sum;
        node = makeOperationOnSubSegment_(node, l, r, [&sum](Node *treeSegment) {
            sum = getSum_(treeSegment);
            return treeSegment;
        });
        return {sum, node};
    }


    static std::pair<long long, Node *> getMin_(Node *node, int l, int r) {
        long long minValue;
        node = makeOperationOnSubSegment_(node, l, r, [&minValue](Node *treeSegment) {
            minValue = getMinValue_(treeSegment);
            return treeSegment;
        });
        return {minValue, node};
    }

    static int indexOf_(Node *root, Node *v) {
        std::vector<Node *> path = {v};
        while (v->parent != nullptr) {
            path.push_back(v->parent);
            v = v->parent;
        }
        std::reverse(path.begin(), path.end());
        int pos = getSize_(root->left) + 1;
        for (int i = 0; i + 1 < path.size(); i++) {
            Node *currentVertex = path[i];
            Node *child = path[i + 1];
            push_(currentVertex);
            if (child == currentVertex->left) {
                pos -= getSize_(currentVertex->left->right) + 1;
            } else {
                pos += getSize_(currentVertex->right->left) + 1;
            }
        }
        return pos;
    }


    static int getMonotoneSuffix_(Node *v, Monotone type) {
        push_(v);
        update_(v);

        if (v == nullptr) {
            return 0;
        }
        if (containsSequence_(v, type)) {
            return getSize_(v);
        }

        int ans = getMonotoneSuffix_(v->right, type);

        if (getSize_(v->right) == ans) {
            if (type == NON_INCREASING && (v->right ? v->value >= v->right->firstValue : true)) {
                ans++;
                update_(v);
                if ((v->left ? v->left->lastValue >= v->value : true)) {
                    ans += getMonotoneSuffix_(v->left, type);
                }
            } else if (type == NON_DECREASING && (v->right ? v->value <= v->right->firstValue : true)) {
                ans++;
                update_(v);
                if ((v->left ? v->left->lastValue <= v->value : true)) {
                    ans += getMonotoneSuffix_(v->left, type);
                }
            }
        }
        return std::max(ans, 1);
    }

    static Node *swapSegments_(Node *root, int l1, int r1, int l2, int r2) {
        auto splitted1 = extractSegment_(root, l1, r1);

        Node *t1 = std::get<0>(splitted1);
        Node *t2 = std::get<1>(splitted1);
        Node *t3 = std::get<2>(splitted1);

        l2 -= r1;
        r2 -= r1;
        auto splitted2 = extractSegment_(t3, l2, r2);
        Node *t4 = std::get<0>(splitted2);
        Node *t5 = std::get<1>(splitted2);
        Node *t6 = std::get<2>(splitted2);

        return merge_(merge_(merge_(merge_(t1, t5), t4), t2), t6);
    }

    static Node *
    getClosestNodeByValue_(Node *node, long long value, const std::function<bool(long long, long long)> &comparator) {
        if (node == nullptr) {
            return nullptr;
        }

        push_(node);

        if (comparator(node->value, value)) {
            Node *rightAns = getClosestNodeByValue_(node->right, value, comparator);
            return rightAns != nullptr ? rightAns : node;
        }

        return getClosestNodeByValue_(node->left, value, comparator);
    }

    static std::pair<Node *, Node *> getMinimalGreater_(Node *node, int l, int r, long long value) {
        Node *minimalGreater;
        node = makeOperationOnSubSegment_(node, l, r, [&minimalGreater, &value](Node *treeSegment) {
            minimalGreater = getClosestNodeByValue_(treeSegment, value, std::greater<>());
            return treeSegment;
        });
        return {minimalGreater, node};
    }

    static std::pair<Node *, Node *> getMaximalLess_(Node *node, int l, int r, long long value) {
        Node *maximalLess;
        node = makeOperationOnSubSegment_(node, l, r, [&maximalLess, &value](Node *treeSegment) {
            maximalLess = getClosestNodeByValue_(treeSegment, value, std::less<>());
            return treeSegment;
        });
        return {maximalLess, node};
    }

    static Node *makePermutation_(Node *root, int l, int r, bool isNext) {
        return makeOperationOnSubSegment_(root, l, r, [&isNext](Node *tree) {
            int monotoneSuffixLength = getMonotoneSuffix_(tree, isNext ? NON_INCREASING : NON_DECREASING);
            int pivotPosition = std::max(1, tree->size - monotoneSuffixLength);
            long long pivotValue = elementAt_(tree, pivotPosition);

            std::pair<Node *, Node *> res;

            if (isNext) {
                res = getMinimalGreater_(tree, pivotPosition + 1, getSize_(tree), pivotValue);
            } else {
                res = getMaximalLess_(tree, pivotPosition + 1, getSize_(tree), pivotValue);
            }
            Node *closestNode = res.first;
            tree = res.second;

            if (closestNode == nullptr) {
                return reverse_(tree, 1, getSize_(tree));
            }

            int indexOfClosestNode = indexOf_(tree, closestNode);

            tree = swapSegments_(tree, pivotPosition, pivotPosition, indexOfClosestNode, indexOfClosestNode);

            return reverse_(tree, pivotPosition + 1, getSize_(tree));
        });
    }


    static Node *nextPermutation_(Node *root, int l, int r) {
        return makePermutation_(root, l, r, true);
    }

    static Node *prevPermutation_(Node *root, int l, int r) {
        return makePermutation_(root, l, r, false);
    }
};

const SplayTree::Query SplayTree::Query::EMPTY = {0, 0};

void readTree(SplayTree &tree, std::istream &in) {
    size_t treeSize;
    in >> treeSize;
    for (size_t i = 0; i < treeSize; i++) {
        long long x;
        in >> x;
        tree.insert(tree.size(), x);
    }
}

void printTree(SplayTree &tree, std::ostream &out) {
    auto treeVector = tree.toVector();
    for (long long element : treeVector) {
        out << element << " ";
    }
}

void processQuery(SplayTree &tree, std::istream &in, std::ostream &out) {
    int type;
    in >> type;
    switch (type) {
        case 1: {
            int l, r;
            in >> l >> r;
            out << tree.getSum(l, r) << "\n";
            break;
        }
        case 2: {
            int pos, x;
            in >> x >> pos;
            tree.insert(pos, x);
            break;
        }
        case 3: {
            int pos;
            in >> pos;
            tree.remove(pos);
            break;
        }
        case 4: {
            int x, l, r;
            in >> x >> l >> r;
            tree.assign(l, r, x);
            break;
        }
        case 5: {
            int x, l, r;
            in >> x >> l >> r;
            tree.add(l, r, x);
            break;
        }
        case 6: {
            int l, r;
            in >> l >> r;
            tree.nextPermutation(l, r);
            break;
        }
        case 7: {
            int l, r;
            in >> l >> r;
            tree.prevPermutation(l, r);
            break;
        }
        default:
            return;
    }
}

void solveProblem(std::istream &in, std::ostream &out) {
    SplayTree tree;
    readTree(tree, in);

    int countOfQueries = 0;
    in >> countOfQueries;

    for (int i = 0; i < countOfQueries; i++) {
        processQuery(tree, in, out);
    }

    printTree(tree, out);
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    solveProblem(std::cin, std::cout);
}
