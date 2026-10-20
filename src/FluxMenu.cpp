#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "BufferConfig.hpp"
#include "FluxTheme.hpp"

using namespace geode::prelude;

// ------------------------------------------------------------ drawing -----
static std::vector<CCPoint> roundedRect(float x0, float y0, float x1, float y1, float r) {
    r = std::max(0.f, std::min({r, (x1 - x0) / 2, (y1 - y0) / 2}));
    std::vector<CCPoint> p;
    if (r < 1.f) return {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
    struct C { float cx, cy, a0; };
    C cs[4] = {{x1 - r, y1 - r, 0}, {x0 + r, y1 - r, 90}, {x0 + r, y0 + r, 180}, {x1 - r, y0 + r, 270}};
    for (auto& c : cs) {
        for (int i = 0; i <= 6; i++) {
            float a = (c.a0 + 15.f * i) * 3.14159265f / 180.f;
            p.push_back({c.cx + r * cosf(a), c.cy + r * sinf(a)});
        }
    }
    return p;
}

static void fillPoly(CCDrawNode* n, const std::vector<CCPoint>& pts, ccColor4F fill, float bw = 0, ccColor4F border = {0, 0, 0, 0}) {
    n->drawPolygon(const_cast<CCPoint*>(pts.data()), pts.size(), fill, bw, border);
}

// Lightning bolt built from two triangles (DrawNode can't fill concave shapes).
// Drawn in a 40x40 box, origin at bottom-left, then scaled.
static void drawBolt(CCDrawNode* n, float size, ccColor4F col) {
    n->clear();
    float s = size / 40.f;
    CCPoint t1[3] = {{30 * s, 40 * s}, {8 * s, 17 * s}, {23 * s, 17 * s}};
    CCPoint t2[3] = {{15 * s, 23 * s}, {32 * s, 23 * s}, {14 * s, 0 * s}};
    n->drawPolygon(t1, 3, col, 0, col);
    n->drawPolygon(t2, 3, col, 0, col);
}

// --------------------------------------------------------------- menu ------
class FluxMenu : public FLAlertLayer {
protected:
    Ref<CCNode> m_root;
    CCDrawNode* m_bg = nullptr;
    CCDrawNode* m_bolt = nullptr;
    CCLabelBMFont* m_title = nullptr;
    CCMenuItemSpriteExtra* m_close = nullptr;
    Ref<CCNode> m_tabBuffer, m_tabLook;
    int m_tab = 0;

    // buffer tab
    std::array<CCMenuItemToggler*, 5> m_stageBtns{};
    CCLabelBMFont* m_summary = nullptr;
    CCLabelBMFont* m_detail = nullptr;
    // look tab
    std::array<Slider*, 8> m_sliders{};
    CCLabelBMFont* m_posLabel = nullptr;
    CCMenuItemSpriteExtra* m_tabBtnA = nullptr;
    CCMenuItemSpriteExtra* m_tabBtnB = nullptr;

    bool init() {
        if (!FLAlertLayer::init(120)) return false;
        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);

        m_root = CCNode::create();
        m_mainLayer->addChild(m_root);

        m_bg = CCDrawNode::create();
        m_root->addChild(m_bg, 0);

        // header
        m_bolt = CCDrawNode::create();
        m_bolt->setPosition({-1000, 0});  // placed in applyTheme
        m_root->addChild(m_bolt, 2);

        m_title = CCLabelBMFont::create("FLUX", "goldFont.fnt");
        m_title->setScale(.6f);
        m_title->setAnchorPoint({0, .5f});
        m_root->addChild(m_title, 2);

        auto closeMenu = CCMenu::create();
        closeMenu->setPosition({0, 0});
        m_root->addChild(closeMenu, 3);
        auto xs = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        xs->setScale(.6f);
        m_close = CCMenuItemSpriteExtra::create(xs, this, menu_selector(FluxMenu::onClose));
        closeMenu->addChild(m_close);

        // tab buttons
        auto tabMenu = CCMenu::create();
        tabMenu->setPosition({0, 0});
        m_root->addChild(tabMenu, 3);
        m_tabBtnA = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Buffer", 70, true, "bigFont.fnt", "GJ_button_01.png", 24.f, .5f),
            this, menu_selector(FluxMenu::onTab));
        m_tabBtnA->setTag(0);
        m_tabBtnA->setPosition({-44, -56});
        m_tabBtnB = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Appearance", 90, true, "bigFont.fnt", "GJ_button_04.png", 24.f, .5f),
            this, menu_selector(FluxMenu::onTab));
        m_tabBtnB->setTag(1);
        m_tabBtnB->setPosition({52, -56});
        tabMenu->addChild(m_tabBtnA);
        tabMenu->addChild(m_tabBtnB);

        this->buildBufferTab();
        this->buildLookTab();
        m_root->addChild(m_tabBuffer, 1);

        this->applyTheme();
        this->refreshBuffer();
        geode::cocos::handleTouchPriority(this);
        return true;
    }

    // ---------------------------------------------------------- tabs ------
    void buildBufferTab() {
        m_tabBuffer = CCNode::create();
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_tabBuffer->addChild(menu);

        for (int i = 0; i < 5; i++) {
            auto on  = ButtonSprite::create(bufcfg::STAGES[i].label, 52, true, "bigFont.fnt", "GJ_button_01.png", 26.f, .5f);
            auto off = ButtonSprite::create(bufcfg::STAGES[i].label, 52, true, "bigFont.fnt", "GJ_button_04.png", 26.f, .5f);
            auto b = CCMenuItemToggler::create(off, on, this, menu_selector(FluxMenu::onStage));
            b->setTag(i);
            b->setPosition({-120.f + i * 60.f, -98});
            menu->addChild(b);
            m_stageBtns[i] = b;
        }

        m_summary = CCLabelBMFont::create("", "goldFont.fnt");
        m_summary->setScale(.65f);
        m_summary->setPosition({0, -135});
        m_tabBuffer->addChild(m_summary);

        m_detail = CCLabelBMFont::create("", "chatFont.fnt");
        m_detail->setScale(.8f);
        m_detail->setOpacity(190);
        m_detail->setPosition({0, -157});
        m_tabBuffer->addChild(m_detail);

        auto hud = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(FluxMenu::onHud), .55f);
        hud->toggle(bufcfg::hudEnabled());
        hud->setPosition({-60, -195});
        menu->addChild(hud);
        auto lbl = CCLabelBMFont::create("Show HUD label", "bigFont.fnt");
        lbl->setScale(.35f);
        lbl->setAnchorPoint({0, .5f});
        lbl->setPosition({-44, -195});
        m_tabBuffer->addChild(lbl);
    }

    void addSlider(CCMenu*, int idx, const char* name, float cx, float y, float v) {
        auto lbl = CCLabelBMFont::create(name, "bigFont.fnt");
        lbl->setScale(.3f);
        lbl->setPosition({cx, y + 13});
        m_tabLook->addChild(lbl);
        auto s = Slider::create(this, menu_selector(FluxMenu::onSlider), .45f);
        s->setPosition({cx, y});
        s->getThumb()->setTag(idx);
        s->setValue(v);
        m_tabLook->addChild(s);
        m_sliders[idx] = s;
    }

    void buildLookTab() {
        m_tabLook = CCNode::create();
        auto& t = flux::theme();
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_tabLook->addChild(menu);

        float L = -82, R = 82;
        addSlider(menu, 0, "Accent R", L, -96,  t.accentR / 255.f);
        addSlider(menu, 1, "Accent G", L, -128, t.accentG / 255.f);
        addSlider(menu, 2, "Accent B", L, -160, t.accentB / 255.f);
        addSlider(menu, 3, "Opacity",  L, -192, flux::unlerp(.3f, 1.f, t.opacity));
        addSlider(menu, 4, "Width",    R, -96,  flux::unlerp(320, 520, t.width));
        addSlider(menu, 5, "Height",   R, -128, flux::unlerp(230, 380, t.height));
        addSlider(menu, 6, "Corners",  R, -160, flux::unlerp(0, 24, t.radius));
        addSlider(menu, 7, "Scale",    R, -192, flux::unlerp(.7f, 1.3f, t.scale));

        auto posBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Position", 70, true, "bigFont.fnt", "GJ_button_04.png", 22.f, .4f),
            this, menu_selector(FluxMenu::onPos));
        posBtn->setPosition({-60, -222});
        menu->addChild(posBtn);
        m_posLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_posLabel->setScale(.7f);
        m_posLabel->setPosition({-5, -222});
        m_posLabel->setAnchorPoint({0, .5f});
        m_tabLook->addChild(m_posLabel);

        auto rst = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Reset", 50, true, "bigFont.fnt", "GJ_button_06.png", 22.f, .4f),
            this, menu_selector(FluxMenu::onReset));
        rst->setPosition({100, -222});
        menu->addChild(rst);
    }

    // -------------------------------------------------------- theming -----
    // Everything is anchored to the TOP of the panel, so changing height
    // never moves the sliders (important while dragging one).
    void applyTheme() {
        auto& t = flux::theme();
        auto win = CCDirector::get()->getWinSize();
        float cx = win.width / 2;
        if (t.pos == 0) cx = t.width * t.scale / 2 + 20;
        if (t.pos == 2) cx = win.width - t.width * t.scale / 2 - 20;
        float top = win.height / 2 + t.height * t.scale / 2;
        m_root->setPosition({cx, top});
        m_root->setScale(t.scale);

        float w = t.width, h = t.height;
        m_bg->clear();
        // glow
        fillPoly(m_bg, roundedRect(-w / 2 - 3, -h - 3, w / 2 + 3, 3, t.radius + 3), flux::accent4(.22f));
        // body
        fillPoly(m_bg, roundedRect(-w / 2, -h, w / 2, 0, t.radius), flux::bg4(t.opacity), 1.5f, flux::accent4(.7f));
        // header strip
        fillPoly(m_bg, roundedRect(-w / 2 + 6, -40, w / 2 - 6, -6, std::min(t.radius, 10.f)), flux::accent4(.14f));

        drawBolt(m_bolt, 22, flux::accent4());
        m_bolt->setPosition({-w / 2 + 16, -34});
        m_title->setPosition({-w / 2 + 44, -23});
        m_title->setColor(flux::accent3());
        m_close->setPosition({w / 2 - 22, -23});

        const char* names[3] = {"Left", "Center", "Right"};
        if (m_posLabel) m_posLabel->setString(names[std::clamp(t.pos, 0, 2)]);
        if (m_summary) m_summary->setColor(flux::accent3());
    }

    void refreshBuffer() {
        int s = bufcfg::stage();
        for (int i = 0; i < 5; i++) m_stageBtns[i]->toggle(i == s);
        m_summary->setString(bufcfg::summary().c_str());
        if (bufcfg::enabled()) {
            m_detail->setString(fmt::format("All: {}   |   Wave: {}",
                bufcfg::frames(bufcfg::allTicks()), bufcfg::frames(bufcfg::waveTicks())).c_str());
        } else {
            m_detail->setString("Buffer disabled, inputs are untouched");
        }
    }

    // ------------------------------------------------------ callbacks -----
    void onTab(CCObject* sender) {
        int tab = sender->getTag();
        if (tab == m_tab) return;
        m_tab = tab;
        m_root->removeChild(m_tabBuffer, false);
        m_root->removeChild(m_tabLook, false);
        m_root->addChild(tab == 0 ? m_tabBuffer.data() : m_tabLook.data(), 1);
        // swap highlight textures
        auto restyle = [](CCMenuItemSpriteExtra* b, const char* text, const char* tex, float w) {
            b->setNormalImage(ButtonSprite::create(text, w, true, "bigFont.fnt", tex, 24.f, .5f));
        };
        restyle(m_tabBtnA, "Buffer",     tab == 0 ? "GJ_button_01.png" : "GJ_button_04.png", 70);
        restyle(m_tabBtnB, "Appearance", tab == 1 ? "GJ_button_01.png" : "GJ_button_04.png", 90);
    }
    void onStage(CCObject* sender) {
        bufcfg::setStage(sender->getTag());
        this->refreshBuffer();
    }
    void onHud(CCObject* sender) {
        bufcfg::setHud(!static_cast<CCMenuItemToggler*>(sender)->isToggled());
    }
    void onSlider(CCObject* sender) {
        auto thumb = static_cast<SliderThumb*>(sender);
        float v = thumb->getValue();
        auto& t = flux::theme();
        switch (thumb->getTag()) {
            case 0: t.accentR = (int)(v * 255); break;
            case 1: t.accentG = (int)(v * 255); break;
            case 2: t.accentB = (int)(v * 255); break;
            case 3: t.opacity = flux::lerp(.3f, 1.f, v); break;
            case 4: t.width   = flux::lerp(320, 520, v); break;
            case 5: t.height  = flux::lerp(230, 380, v); break;
            case 6: t.radius  = flux::lerp(0, 24, v); break;
            case 7: t.scale   = flux::lerp(.7f, 1.3f, v); break;
        }
        flux::save();
        this->applyTheme();
    }
    void onPos(CCObject*) {
        auto& t = flux::theme();
        t.pos = (t.pos + 1) % 3;
        flux::save();
        this->applyTheme();
    }
    void onReset(CCObject*) {
        flux::reset();
        auto& t = flux::theme();
        float v[8] = {t.accentR / 255.f, t.accentG / 255.f, t.accentB / 255.f,
                      flux::unlerp(.3f, 1.f, t.opacity), flux::unlerp(320, 520, t.width),
                      flux::unlerp(230, 380, t.height), flux::unlerp(0, 24, t.radius),
                      flux::unlerp(.7f, 1.3f, t.scale)};
        for (int i = 0; i < 8; i++) m_sliders[i]->setValue(v[i]);
        this->applyTheme();
    }
    void onClose(CCObject*) {
        this->setKeypadEnabled(false);
        this->removeFromParentAndCleanup(true);
    }
    void keyBackClicked() override { this->onClose(nullptr); }

public:
    static FluxMenu* create() {
        auto ret = new FluxMenu();
        if (ret->init()) { ret->autorelease(); return ret; }
        delete ret;
        return nullptr;
    }
};

// ------------------------------------------------------- pause button ------
class $modify(FluxPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        flux::load();
        auto spr = ButtonSprite::create("Flux", 50, true, "bigFont.fnt", "GJ_button_01.png", 30.f, .6f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FluxPauseLayer::onFlux));
        btn->setID("flux-button"_spr);
        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        } else {
            auto m = CCMenu::create();
            m->addChild(btn);
            m->setPosition({CCDirector::get()->getWinSize().width - 50.f, 40.f});
            this->addChild(m);
        }
    }
    void onFlux(CCObject*) { FluxMenu::create()->show(); }
};

// --------------------------------------------------------- HUD label -------
class $modify(FluxPlayLayer, PlayLayer) {
    struct Fields { CCLabelBMFont* hud = nullptr; };

    bool init(GJGameLevel* lvl, bool a, bool b) {
        if (!PlayLayer::init(lvl, a, b)) return false;
        flux::load();
        auto size = CCDirector::get()->getWinSize();
        m_fields->hud = CCLabelBMFont::create("", "bigFont.fnt");
        m_fields->hud->setScale(.3f);
        m_fields->hud->setOpacity(150);
        m_fields->hud->setAnchorPoint({1, 0});
        m_fields->hud->setPosition({size.width - 6.f, 6.f});
        this->addChild(m_fields->hud, 100);
        this->schedule(schedule_selector(FluxPlayLayer::updateHud), .1f);
        return true;
    }
    void updateHud(float) {
        auto h = m_fields->hud;
        if (!h) return;
        h->setVisible(bufcfg::hudEnabled());
        h->setColor(flux::accent3());
        h->setString(fmt::format("Flux: {}", bufcfg::summary()).c_str());
    }
};
