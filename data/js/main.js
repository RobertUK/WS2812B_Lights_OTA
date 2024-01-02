!(function (e) {
    var t,
        o = e(window),
        i = e("body"),
        s = e("#wrapper"),
        n = e("#header"),
        a = e("#footer"),
        l = e("#main"),
        r = l.children("article");
    (breakpoints({ xlarge: ["1281px", "1680px"], large: ["981px", "1280px"], medium: ["737px", "980px"], small: ["481px", "736px"], xsmall: ["361px", "480px"], xxsmall: [null, "360px"] }),
    o.on("load", function () {
        window.setTimeout(function () {
            i.removeClass("is-preload");
        }, 100);
    }),
    "ie" == browser.name) &&
        o
            .on("resize.flexbox-fix", function () {
                clearTimeout(t),
                    (t = setTimeout(function () {
                        s.prop("scrollHeight") > o.height() ? s.css("height", "auto") : s.css("height", "100vh");
                    }, 250));
            })
            .triggerHandler("resize.flexbox-fix");
    var c = n.children("nav"),
        h = c.find("li");
    h.length % 2 == 0 && (c.addClass("use-middle"), h.eq(h.length / 2).addClass("is-middle"));
    var u = 325,
        d = !1;
    if (
        ((l._show = function (e, t) {
            var s = r.filter("#" + e);
            if (0 != s.length) {
                if (d || (void 0 !== t && !0 === t))
                    return (
                        i.addClass("is-switching"),
                        i.addClass("is-article-visible"),
                        r.removeClass("active"),
                        n.hide(),
                        a.hide(),
                        l.show(),
                        s.show(),
                        s.addClass("active"),
                        (d = !1),
                        void setTimeout(
                            function () {
                                i.removeClass("is-switching");
                            },
                            t ? 1e3 : 0
                        )
                    );
                if (((d = !0), i.hasClass("is-article-visible"))) {
                    var c = r.filter(".active");
                    c.removeClass("active"),
                        setTimeout(function () {
                            c.hide(),
                                s.show(),
                                setTimeout(function () {
                                    s.addClass("active"),
                                        o.scrollTop(0).triggerHandler("resize.flexbox-fix"),
                                        setTimeout(function () {
                                            d = !1;
                                        }, u);
                                }, 25);
                        }, u);
                } else
                    i.addClass("is-article-visible"),
                        setTimeout(function () {
                            n.hide(),
                                a.hide(),
                                l.show(),
                                s.show(),
                                setTimeout(function () {
                                    s.addClass("active"),
                                        o.scrollTop(0).triggerHandler("resize.flexbox-fix"),
                                        setTimeout(function () {
                                            d = !1;
                                        }, u);
                                }, 25);
                        }, u);
            }
        }),
        (l._hide = function (e) {
            var t = r.filter(".active");
            if (i.hasClass("is-article-visible")) {
                if ((void 0 !== e && !0 === e && history.pushState(null, null, "#"), d))
                    return (
                        i.addClass("is-switching"),
                        t.removeClass("active"),
                        t.hide(),
                        l.hide(),
                        a.show(),
                        n.show(),
                        i.removeClass("is-article-visible"),
                        (d = !1),
                        i.removeClass("is-switching"),
                        void o.scrollTop(0).triggerHandler("resize.flexbox-fix")
                    );
                (d = !0),
                    t.removeClass("active"),
                    setTimeout(function () {
                        t.hide(),
                            l.hide(),
                            a.show(),
                            n.show(),
                            setTimeout(function () {
                                i.removeClass("is-article-visible"),
                                    o.scrollTop(0).triggerHandler("resize.flexbox-fix"),
                                    setTimeout(function () {
                                        d = !1;
                                    }, u);
                            }, 25);
                    }, u);
            }
        }),
        r.each(function () {
            var t = e(this);
            e('<div class="close">Close</div>')
                .appendTo(t)
                .on("click", function () {
                    location.hash = "";
                }),
                t.on("click", function (e) {
                    e.stopPropagation();
                });
        }),
        i.on("click", function (e) {
            i.hasClass("is-article-visible") && l._hide(!0);
        }),
        o.on("keyup", function (e) {
            if (27 === e.keyCode) i.hasClass("is-article-visible") && l._hide(!0);
        }),
        o.on("hashchange", function (e) {
            "" == location.hash || "#" == location.hash ? (e.preventDefault(), e.stopPropagation(), l._hide()) : r.filter(location.hash).length > 0 && (e.preventDefault(), e.stopPropagation(), l._show(location.hash.substr(1)));
        }),
        "scrollRestoration" in history)
    )
        history.scrollRestoration = "manual";
    else {
        var f = 0,
            p = 0,
            v = e("html,body");
        o.on("scroll", function () {
            (f = p), (p = v.scrollTop());
        }).on("hashchange", function () {
            o.scrollTop(f);
        });
    }
    l.hide(),
        r.hide(),
        "" != location.hash &&
            "#" != location.hash &&
            o.on("load", function () {
                l._show(location.hash.substr(1), !0);
            });
    let x = new XMLHttpRequest();
    x.open("GET", "/on"),
        (x.onreadystatechange = function () {
            4 === x.readyState && (200 === x.status ? (console.log(x.responseText), e.notify({ title: h5, button: "YES !" }, { style: "foo", autoHide: !1, clickToHide: !1 })) : console.log("HTTP error", x.status, x.statusText));
        });
    let g = new XMLHttpRequest();
    g.open("GET", "/off"),
        (g.onreadystatechange = function () {
            4 === g.readyState && (200 === g.status ? console.log(g.responseText) : console.log("HTTP error", g.status, g.statusText));
        });
    let T = new XMLHttpRequest();
    T.open("GET", "/on25"),
        (T.onreadystatechange = function () {
            4 === T.readyState && (200 === T.status ? console.log(xhrOn25responseText) : (console.log("HTTP error", T.status, T.statusText), e.notify("Simple Notify")));
        });
    let m = new XMLHttpRequest();
    m.open("GET", "/update"),
        (m.onreadystatechange = function () {
            4 === m.readyState && (200 === m.status ? (console.log(g.responseText), e.notify("Simple Notify")) : (console.log("HTTP error", m.status, m.statusText), e.notify("Simple Notify")));
        }),
        m.send(),
        e("update").click(function () {
            e.notify("Simple Notify");
        });
})(jQuery);
