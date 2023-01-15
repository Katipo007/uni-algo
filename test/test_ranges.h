/* Test for Unicode Algorithms Implementation.
 * License: Public Domain or MIT - choose whatever you want.
 * See LICENSE.md */

// If it fails a compiler messed up the UTF-8 encoding of this file.
// If the compiler is MSVC then /utf-8 command line option must be used.
static_assert(U'㋡' == 0x32E1);

test_constexpr bool test_ranges()
{
    TESTX("789" == (std::u32string_view{U"123789"} | uni::views::drop(3) | uni::ranges::to_utf8<std::string>()));
    TESTX("123" == (std::u32string_view{U"123789"} | uni::views::take(3) | uni::ranges::to_utf8<std::string>()));
    TESTX("987321" == (std::u32string_view{U"123789"} | uni::views::reverse | uni::ranges::to_utf8<std::string>()));
    TESTX("123" == (std::u32string_view{U"   123"} | uni::views::filter([](char32_t c) { return c != ' '; }) | uni::ranges::to_utf8<std::string>()));
    // See comments in uni_algo/ranges.h -> uni::ranges::to_utf8 adaptor
    // REMINDER: static_assert(std::is_same_v<decltype((char32_t)1+1), char32_t>) is false
    TESTX("789" == (std::u32string_view{U"123"} | uni::views::transform([](char32_t c) -> char32_t { return c + 6; }) | uni::ranges::to_utf8<std::string>()));
    TESTX("789" == (std::u32string_view{U"123"} | uni::views::transform([](char32_t c) { return c + 6; }) | uni::ranges::to_utf8<std::string>()));

    //std::string result = std::string_view{"12345678900"}
    std::string result = std::string_view{"アイウエオカキクケココ"}
             | uni::views::utf8
             | uni::views::reverse | uni::views::take(7) | uni::views::reverse
             | uni::views::drop(2) | uni::views::reverse
             | uni::ranges::to_utf8<std::string>();
    //TESTX{result == "00987"};
    TESTX(result == "ココケクキ");

    // All of the following must be equivalent

    auto range1 = std::string_view{"アイウエオカキクケココ"}
             | uni::views::utf8
             | uni::views::reverse | uni::views::take(7) | uni::views::reverse
             | uni::views::drop(2) | uni::views::reverse;
    std::u32string result1{range1.begin(), range1.end()};
    TESTX(result1 == U"ココケクキ");

    auto range2 = uni::views::reverse(
        uni::views::drop(
            uni::views::reverse(
                uni::views::take(
                    uni::views::reverse(
                        uni::views::utf8(std::string_view{"アイウエオカキクケココ"})
                ), 7)
        ), 2)
    );
    std::u32string result2{range2.begin(), range2.end()};
    TESTX(result2 == U"ココケクキ");

    auto range3 = uni::ranges::reverse_view{
        uni::ranges::drop_view{
            uni::ranges::reverse_view{
                uni::ranges::take_view{
                    uni::ranges::reverse_view{
                        uni::ranges::utf8_view{std::string_view{"アイウエオカキクケココ"}}
                }, 7}
        }, 2}
    };
    std::u32string result3{range3.begin(), range3.end()};
    TESTX(result3 == U"ココケクキ");

    return true;
}

// Custom allocator for the following tests
template<class T>
class alloc_range
{
public:
    using value_type = T;

    // NOTE: Does not compile in MSVC Debug without this constructor but compiles in Release
    template <class U>
    test_constexpr alloc_range(const alloc_range<U>&) {}

    test_constexpr alloc_range() = default;
    test_constexpr T* allocate(std::size_t n)
    {
        T* p = new T[n];
        //T* p = static_cast<T*>(std::malloc(n * sizeof(T)));
        //std::cout << "Alloc  : " << n << " bytes at " << static_cast<void*>(p) << '\n';
        return p;
    }
    test_constexpr void deallocate(T* p, std::size_t n)
    {
        (void)n;
        //std::cout << "Dealloc: " << n << " bytes at " << static_cast<void*>(p) << '\n';
        //std::free(static_cast<void*>(p));
        delete[] p;
    }
};

test_constexpr bool test_ranges_to()
{
    TESTX("123" == (std::u32string_view{U"123"} | uni::ranges::to_utf8<std::string>()));
    TESTX("123" == uni::ranges::to_utf8<std::string>(std::u32string_view{U"123"}));
    TESTX(u"123" == (std::u32string_view{U"123"} | uni::ranges::to_utf16<std::u16string>()));
    TESTX(u"123" == uni::ranges::to_utf16<std::u16string>(std::u32string_view{U"123"}));

    TESTX("123" == (std::u32string_view{U"123"} | uni::ranges::to_utf8_reserve<std::string>(3)));
    TESTX("123" == uni::ranges::to_utf8_reserve<std::string>(std::u32string_view{U"123"}, 3));
    TESTX(u"123" == (std::u32string_view{U"123"} | uni::ranges::to_utf16_reserve<std::u16string>(3)));
    TESTX(u"123" == uni::ranges::to_utf16_reserve<std::u16string>(std::u32string_view{U"123"}, 3));

    TESTX(std::string_view("12", 3) == (U"12" | uni::ranges::to_utf8<std::string>()));
    TESTX(std::string_view("12", 3) == uni::ranges::to_utf8<std::string>(U"12"));
    TESTX(std::u16string_view(u"12", 3) == (U"12" | uni::ranges::to_utf16<std::u16string>()));
    TESTX(std::u16string_view(u"12", 3) == uni::ranges::to_utf16<std::u16string>(U"12"));

    TESTX(std::string_view("12", 3) == (U"12" | uni::ranges::to_utf8_reserve<std::string>(3)));
    TESTX(std::string_view("12", 3) == uni::ranges::to_utf8_reserve<std::string>(U"12", 3));
    TESTX(std::u16string_view(u"12", 3) == (U"12" | uni::ranges::to_utf16_reserve<std::u16string>(3)));
    TESTX(std::u16string_view(u"12", 3) == uni::ranges::to_utf16_reserve<std::u16string>(U"12", 3));

    // With custom allocator

    using string = std::basic_string<char, std::char_traits<char>, alloc_range<char>>;
    using u16string = std::basic_string<char16_t, std::char_traits<char16_t>, alloc_range<char16_t>>;

    TESTX("123" == (std::u32string_view{U"123"} | uni::ranges::to_utf8<string>(alloc_range<char>{})));
    TESTX("123" == uni::ranges::to_utf8<string>(std::u32string_view{U"123"}, alloc_range<char>{}));
    TESTX(u"123" == (std::u32string_view{U"123"} | uni::ranges::to_utf16<u16string>(alloc_range<char16_t>{})));
    TESTX(u"123" == uni::ranges::to_utf16<u16string>(std::u32string_view{U"123"}, alloc_range<char16_t>{}));

    TESTX("123" == (std::u32string_view{U"123"} | uni::ranges::to_utf8_reserve<string>(3, alloc_range<char>{})));
    TESTX("123" == uni::ranges::to_utf8_reserve<string>(std::u32string_view{U"123"}, 3, alloc_range<char>{}));
    TESTX(u"123" == (std::u32string_view{U"123"} | uni::ranges::to_utf16_reserve<u16string>(3, alloc_range<char16_t>{})));
    TESTX(u"123" == uni::ranges::to_utf16_reserve<u16string>(std::u32string_view{U"123"}, 3, alloc_range<char16_t>{}));

    TESTX(std::string_view("12", 3) == (U"12" | uni::ranges::to_utf8<string>(alloc_range<char>{})));
    TESTX(std::string_view("12", 3) == uni::ranges::to_utf8<string>(U"12", alloc_range<char>{}));
    TESTX(std::u16string_view(u"12", 3) == (U"12" | uni::ranges::to_utf16<u16string>(alloc_range<char16_t>{})));
    TESTX(std::u16string_view(u"12", 3) == uni::ranges::to_utf16<u16string>(U"12", alloc_range<char16_t>{}));

    TESTX(std::string_view("12", 3) == (U"12" | uni::ranges::to_utf8_reserve<string>(3, alloc_range<char>{})));
    TESTX(std::string_view("12", 3) == uni::ranges::to_utf8_reserve<string>(U"12", 3, alloc_range<char>{}));
    TESTX(std::u16string_view(u"12", 3) == (U"12" | uni::ranges::to_utf16_reserve<u16string>(3, alloc_range<char16_t>{})));
    TESTX(std::u16string_view(u"12", 3) == uni::ranges::to_utf16_reserve<u16string>(U"12", 3, alloc_range<char16_t>{}));

    return true;
}

test_constexpr bool test_ranges_ctad()
{
    // Skip constexpr test because uses TESTX inside class my_string
    // TODO: It can be fixed
#ifndef TEST_MODE_CONSTEXPR

    // If everything works properly none of these constructors and operators must be called
    class my_string
    {
    private:
        std::string str;
    public:
        my_string() = default;
        explicit my_string(const char* s) { str = s; }
        my_string(const my_string& s) { TESTX(false); std::cout << "copy contruct" << '\n'; str = s.str; }
        my_string(my_string&& s) { TESTX(false); std::cout << "move contruct" << '\n'; str = std::move(s.str); }
        my_string& operator=(const my_string& s) { TESTX(false); std::cout << "copy operator" << '\n'; str = s.str; return *this; }
        my_string& operator=(my_string&& s) { TESTX(false); std::cout << "move operator" << '\n'; str = std::move(s.str); return *this; }
        auto begin() const { return str.begin(); }
        auto end() const { return str.end(); }
    };

    my_string str{"12345678900"};
    TESTX(*(str | uni::views::all).begin() == '1');
    TESTX(*(str | uni::views::utf8).begin() == U'1');
    TESTX(*(str | uni::views::reverse).begin() == '0');
    TESTX(*(str | uni::views::take(1)).begin() == '1');
    TESTX(*(str | uni::views::drop(1)).begin() == '2');
    TESTX(*(str | uni::views::filter([](char c) { return c != '1'; })).begin() == '2');
    TESTX(*(str | uni::views::transform([](char) { return '7'; })).begin() == '7');

    // It works this way too (it breaks compilation if CTAD is wrong)
    // so for views that won't work with std::string just the simple test
    TESTX(*(u"123" | uni::views::utf16).begin() == u'1');

    TESTX(*(U"123" | uni::views::norm::nfc).begin() == U'1');
    TESTX(*(U"123" | uni::views::norm::nfd).begin() == U'1');
    TESTX(*(U"123" | uni::views::norm::nfkc).begin() == U'1');
    TESTX(*(U"123" | uni::views::norm::nfkd).begin() == U'1');

    TESTX(*("123" | uni::views::grapheme::utf8).begin() == "1");
    TESTX(*("123 789" | uni::views::word::utf8).begin() == "123");
    TESTX(*(" 123 789" | uni::views::word_only::utf8).begin() == "123");

    TESTX(*(u"123" | uni::views::grapheme::utf8).begin() == u"1");
    TESTX(*(u"123 789" | uni::views::word::utf8).begin() == u"123");
    TESTX(*(u" 123 789" | uni::views::word_only::utf8).begin() == u"123");

#endif // TEST_MODE_CONSTEXPR

    return true;
}

test_constexpr bool test_ranges_static_assert()
{
    // NOTE: When ranges from Clang with libc++ (15.0.5 -fexperimental-library) is used
    // then this test doesn't compile because of P1814R0 (still not implemented in Clang 15) it seems.
    // Always works fine with our ranges so use !defined(__cpp_lib_ranges) first.
    // This test is only needed for our ranges btw because we test here that our ranges are implemented properly so it's fine.
#if !defined(__cpp_lib_ranges) || !defined(__clang__)

    static_assert(std::is_same_v<decltype("12345" | uni::views::reverse),
            decltype(uni::ranges::reverse_view{uni::ranges::ref_view{"00000"}})>); // must be ref_view here
    static_assert(std::is_same_v<decltype(std::string_view{"12345"} | uni::views::reverse),
            decltype(uni::ranges::reverse_view{std::string_view{}})>); // must not be ref_view here
#if !defined(__cpp_lib_ranges) || (__cpp_lib_ranges >= 202110L) // if owning_view is available
    static_assert(std::is_same_v<decltype(std::string{"12345"} | uni::views::reverse),
            decltype(uni::ranges::reverse_view{uni::ranges::owning_view{std::string{}}})>); // must be owning_view here
#endif
    // std::decay_t TEST
    /*
        // std::decay_t TEST (see our all view in uni_algo/ranges.h for details)
        // Test: std::is_base_of_v<detail::rng::view_base, std::decay_t<R>>
        const std::string_view str_view{"12345"};
        const auto my_view = str_view | uni::views::reverse;
        my_view | uni::views::drop(1) = 0; // must not be ref_view here
        // Test: std::is_same_v<std::basic_string_view<range_v>, std::decay_t<R>>
        const std::string_view str_view{"12345"};
        str_view | uni::views::utf8 = 0; // must not be ref_view here
    */
    { // Test: std::is_base_of_v<detail::rng::view_base, std::decay_t<R>>
        const std::string_view str_view{"12345"};
        auto my_view = str_view | uni::views::reverse;
        static_assert(std::is_same_v<decltype(my_view | uni::views::drop(1)),
                decltype(uni::ranges::drop_view{uni::ranges::reverse_view{std::string_view{}}, 1})>); // must not be ref_view here
    }
    { // Test: std::is_same_v<std::basic_string_view<range_v>, std::decay_t<R>>
        const std::string_view str_view{"12345"};
        static_assert(std::is_same_v<decltype(str_view | uni::views::reverse),
                decltype(uni::ranges::reverse_view{std::string_view{}})>); // must not be ref_view here
    }
    // Other tests
#if !defined(__cpp_lib_ranges) || (__cpp_lib_ranges >= 202110L) // if owning_view is available
    static_assert(std::is_same_v<decltype(std::string{"12345"} | uni::views::reverse | uni::views::drop(1)),
            decltype(uni::ranges::drop_view{uni::ranges::reverse_view{uni::ranges::owning_view{std::string{}}}, 1})>); // must be owning_view here
#endif
    {
        std::string str{"12345"};
        static_assert(std::is_same_v<decltype(str | uni::views::reverse),
                decltype(uni::ranges::reverse_view{uni::ranges::ref_view{str}})>); // must be ref_view here
    }
    {
        const std::string str{"12345"};
        static_assert(std::is_same_v<decltype(str | uni::views::reverse),
                decltype(uni::ranges::reverse_view{uni::ranges::ref_view{str}})>); // must be ref_view here
    }
    {
        std::string_view str_view{"12345"};
        static_assert(std::is_same_v<decltype(str_view | uni::views::reverse),
                decltype(uni::ranges::reverse_view{str_view})>); // must not be ref_view here
    }
    {
        const std::string_view str_view{"12345"};
        static_assert(std::is_same_v<decltype(str_view | uni::views::reverse),
                decltype(uni::ranges::reverse_view{str_view})>); // must not be ref_view here
    }

#endif // Clang

    // Test uni::detail::rng::iter_pointer_t
    static_assert(std::is_same_v<char*, uni::detail::rng::iter_pointer_t<decltype(std::string{"123"}.begin())>>);
    static_assert(std::is_same_v<const char*, uni::detail::rng::iter_pointer_t<decltype(std::string{"123"}.cbegin())>>);
    static_assert(std::is_same_v<const char*, uni::detail::rng::iter_pointer_t<decltype(std::string_view{"123"}.begin())>>);
    static_assert(std::is_same_v<const char*, uni::detail::rng::iter_pointer_t<decltype(std::string_view{"123"}.cbegin())>>);

    return true;
}
