template <class F, class... Args>
void for_each_argument(F f, Args&&... args) {
    [](...){}((f(std::forward<Args>(args)), 0)...);
}