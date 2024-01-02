!(function (t) {
    (t.fn.navList = function () {
        return (
            ($a = t(this).find("a")),
            (b = []),
            $a.each(function () {
                var e = t(this),
                    a = Math.max(0, e.parents("li").length - 1),
                    l = e.attr("href"),
                    i = e.attr("target");
                b.push('<a class="link depth-' + a + '"' + (void 0 !== i && "" != i ? ' target="' + i + '"' : "") + (void 0 !== l && "" != l ? ' href="' + l + '"' : "") + '><span class="indent-' + a + '"></span>' + e.text() + "</a>");
            }),
            b.join("")
        );
    }),
        (t.fn.panel = function (e) {
            if (0 == this.length) return i;
            if (this.length > 1) {
                for (var a = 0; a < this.length; a++) t(this[a]).panel(e);
                return i;
            }
            var l,
                i = t(this),
                r = t("body"),
                o = t(window),
                n = i.attr("id");
            return (
                "jQuery" != typeof (l = t.extend({ delay: 0, hideOnClick: !1, hideOnEscape: !1, hideOnSwipe: !1, resetScroll: !1, resetForms: !1, side: null, target: i, visibleClass: "visible" }, e)).target && (l.target = t(l.target)),
                (i._hide = function (t) {
                    l.target.hasClass(l.visibleClass) &&
                        (t && (t.preventDefault(), t.stopPropagation()),
                        l.target.removeClass(l.visibleClass),
                        window.setTimeout(function () {
                            l.resetScroll && i.scrollTop(0),
                                l.resetForms &&
                                    i.find("form").each(function () {
                                        this.reset();
                                    });
                        }, l.delay));
                }),
                i.css("-ms-overflow-style", "-ms-autohiding-scrollbar").css("-webkit-overflow-scrolling", "touch"),
                l.hideOnClick &&
                    (i.find("a").css("-webkit-tap-highlight-color", "rgba(0,0,0,0)"),
                    i.on("click", "a", function (e) {
                        var a = t(this),
                            r = a.attr("href"),
                            o = a.attr("target");
                        r &&
                            "#" != r &&
                            "" != r &&
                            r != "#" + n &&
                            (e.preventDefault(),
                            e.stopPropagation(),
                            i._hide(),
                            window.setTimeout(function () {
                                "_blank" == o ? window.open(r) : (window.location.href = r);
                            }, l.delay + 10));
                    })),
                i.on("touchstart", function (t) {
                    (i.touchPosX = t.originalEvent.touches[0].pageX), (i.touchPosY = t.originalEvent.touches[0].pageY);
                }),
                i.on("touchmove", function (t) {
                    if (null !== i.touchPosX && null !== i.touchPosY) {
                        var e = i.touchPosX - t.originalEvent.touches[0].pageX,
                            a = i.touchPosY - t.originalEvent.touches[0].pageY,
                            r = i.outerHeight(),
                            o = i.get(0).scrollHeight - i.scrollTop();
                        if (l.hideOnSwipe) {
                            var n = !1;
                            switch (l.side) {
                                case "left":
                                    n = a < 20 && a > -20 && e > 50;
                                    break;
                                case "right":
                                    n = a < 20 && a > -20 && e < -50;
                                    break;
                                case "top":
                                    n = e < 20 && e > -20 && a > 50;
                                    break;
                                case "bottom":
                                    n = e < 20 && e > -20 && a < -50;
                            }
                            if (n) return (i.touchPosX = null), (i.touchPosY = null), i._hide(), !1;
                        }
                        ((0 > i.scrollTop() && a < 0) || (o > r - 2 && o < r + 2 && a > 0)) && (t.preventDefault(), t.stopPropagation());
                    }
                }),
                i.on("click touchend touchstart touchmove", function (t) {
                    t.stopPropagation();
                }),
                i.on("click", 'a[href="#' + n + '"]', function (t) {
                    t.preventDefault(), t.stopPropagation(), l.target.removeClass(l.visibleClass);
                }),
                r.on("click touchend", function (t) {
                    i._hide(t);
                }),
                r.on("click", 'a[href="#' + n + '"]', function (t) {
                    t.preventDefault(), t.stopPropagation(), l.target.toggleClass(l.visibleClass);
                }),
                l.hideOnEscape &&
                    o.on("keydown", function (t) {
                        27 == t.keyCode && i._hide(t);
                    }),
                i
            );
        }),
        (t.fn.placeholder = function () {
            if (void 0 !== document.createElement("input").placeholder) return t(this);
            if (0 == this.length) return a;
            if (this.length > 1) {
                for (var e = 0; e < this.length; e++) t(this[e]).placeholder();
                return a;
            }
            var a = t(this);
            return (
                a
                    .find("input[type=text],textarea")
                    .each(function () {
                        var e = t(this);
                        ("" == e.val() || e.val() == e.attr("placeholder")) && e.addClass("polyfill-placeholder").val(e.attr("placeholder"));
                    })
                    .on("blur", function () {
                        var e = t(this);
                        e.attr("name").match(/-polyfill-field$/) || "" != e.val() || e.addClass("polyfill-placeholder").val(e.attr("placeholder"));
                    })
                    .on("focus", function () {
                        var e = t(this);
                        e.attr("name").match(/-polyfill-field$/) || e.val() != e.attr("placeholder") || e.removeClass("polyfill-placeholder").val("");
                    }),
                a.find("input[type=password]").each(function () {
                    var e = t(this),
                        a = t(
                            t("<div>")
                                .append(e.clone())
                                .remove()
                                .html()
                                .replace(/type="password"/i, 'type="text"')
                                .replace(/type=password/i, "type=text")
                        );
                    "" != e.attr("id") && a.attr("id", e.attr("id") + "-polyfill-field"),
                        "" != e.attr("name") && a.attr("name", e.attr("name") + "-polyfill-field"),
                        a.addClass("polyfill-placeholder").val(a.attr("placeholder")).insertAfter(e),
                        "" == e.val() ? e.hide() : a.hide(),
                        e.on("blur", function (t) {
                            t.preventDefault();
                            var a = e.parent().find("input[name=" + e.attr("name") + "-polyfill-field]");
                            "" == e.val() && (e.hide(), a.show());
                        }),
                        a
                            .on("focus", function (t) {
                                t.preventDefault();
                                var e = a.parent().find("input[name=" + a.attr("name").replace("-polyfill-field", "") + "]");
                                a.hide(), e.show().focus();
                            })
                            .on("keypress", function (t) {
                                t.preventDefault(), a.val("");
                            });
                }),
                a
                    .on("submit", function () {
                        a.find("input[type=text],input[type=password],textarea").each(function (e) {
                            var a = t(this);
                            a.attr("name").match(/-polyfill-field$/) && a.attr("name", ""), a.val() == a.attr("placeholder") && (a.removeClass("polyfill-placeholder"), a.val(""));
                        });
                    })
                    .on("reset", function (e) {
                        e.preventDefault(),
                            a.find("select").val(t("option:first").val()),
                            a.find("input,textarea").each(function () {
                                var e,
                                    a = t(this);
                                switch ((a.removeClass("polyfill-placeholder"), this.type)) {
                                    case "submit":
                                    case "reset":
                                        break;
                                    case "password":
                                        a.val(a.attr("defaultValue")), (e = a.parent().find("input[name=" + a.attr("name") + "-polyfill-field]")), "" == a.val() ? (a.hide(), e.show()) : (a.show(), e.hide());
                                        break;
                                    case "checkbox":
                                    case "radio":
                                        a.attr("checked", a.attr("defaultValue"));
                                        break;
                                    case "text":
                                    case "textarea":
                                        a.val(a.attr("defaultValue")), "" == a.val() && (a.addClass("polyfill-placeholder"), a.val(a.attr("placeholder")));
                                        break;
                                    default:
                                        a.val(a.attr("defaultValue"));
                                }
                            });
                    }),
                a
            );
        }),
        (t.prioritize = function (e, a) {
            var l = "__prioritize";
            "jQuery" != typeof e && (e = t(e)),
                e.each(function () {
                    var e,
                        i = t(this),
                        r = i.parent();
                    if (0 != r.length) {
                        if (i.data(l)) {
                            if (a) return;
                            (e = i.data(l)), i.insertAfter(e), i.removeData(l);
                        } else {
                            if (!a || 0 == (e = i.prev()).length) return;
                            i.prependTo(r), i.data(l, e);
                        }
                    }
                });
        });
})(jQuery);
