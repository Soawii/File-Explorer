#include "FileExplorerView.hpp"
#include "../engine/EventBus.hpp"
#include "../UI/TextElement.hpp"
#include "../util/util.hpp"
#include "../UI/Slider.hpp"
#include "../UI/Button.hpp"
#include "../filesystem/FileSystem.hpp"
#include "../engine/sync.hpp"
#include <iostream>
#include <iomanip>

using namespace ui;

FileExplorerView::FileExplorerView(UIManager* ui, FileExplorerModel& model) 
: m_ui(ui), m_model(model) 
{
    m_stringDirectory += static_cast<sf::Uint32>(0x1F5BF);
    m_stringFile += static_cast<sf::Uint32>(0x1F4C4);

    m_win = m_ui->m_windowElement;

    m_ui->m_modalBackdrop = new UIElement();
    m_ui->addElement(m_ui->m_modalBackdrop, m_win, m_win, "modal_backdrop", {});
    m_ui->m_modalBackdrop->m_vars.size.relative.setAll({1.0f, 1.0f});
    m_ui->m_modalBackdrop->m_vars.color.setAll(Color(0.0f,0.0f,0.0f,0.2f));
    m_ui->m_modalBackdrop->m_vars.opacity.setState(UIElementState::DISABLED, 0.0f);
    m_ui->m_modalBackdrop->setState(UIElementState::DISABLED);
    m_ui->m_modalBackdrop->m_vars.opacity.setDuration(0.2f);
    m_ui->m_modalBackdrop->m_vars.opacity.setTimingFunction(TimingFunctions::easeOutQuad);
    m_ui->m_modalBackdrop->m_context->z_index = 200;
    m_ui->m_modalBackdrop->m_context->propogateScroll = false;
    m_ui->m_modalBackdrop->m_context->onClick.push_back([this](MouseContext& m) {
        m_ui->closeModal();
    });

    m_layout = new Flex();
    m_ui->addElement(m_layout, m_win, m_win, "layout", {});
    m_layout->m_vars.size.relative.setAll({1.0f, 1.0f});
    m_layout->m_vars.color.setAll(Color(200,200,200));
    m_layout->m_direction = FlexDirection::vertical;
    m_layout->m_context->dontUpdateYourself = true;
    m_layout->m_context->statelessElement = true;
    m_layout->m_gap = 1.0f;

    m_top_bar = new Flex();
    m_ui->addElement(m_top_bar, m_layout, m_layout, "top_bar", {});
    m_top_bar->m_vars.size.relative.setAll({1.0f, 0.0f});
    m_top_bar->m_vars.size.absolute.setAll({0.0f, 60.0f});
    m_top_bar->m_vars.color.setAll(Color(230,230,233));
    m_top_bar->m_vars.padding.setAll(8.0f);
    m_top_bar->m_direction = FlexDirection::horizontal;
    m_top_bar->m_gap = 8.0f;
    m_top_bar->m_context->dontUpdateYourself = true;
    m_top_bar->m_context->statelessElement = true;

    m_file_path = new Flex();
    m_ui->addElement(m_file_path, m_top_bar, m_top_bar, "file_path", {});
    m_file_path->m_vars.size.relative.setAll({0.0f, 1.0f});
    m_file_path->m_vars.color.setAll(Color(245,245,245));
    m_file_path->m_vars.borderColor.setAll(Color(200,200,200));
    m_file_path->m_vars.borderWidth.setAll(1.0f);
    m_file_path->m_vars.flex.setAll(1.0f);
    m_file_path->m_direction = FlexDirection::horizontal;
    m_file_path->m_align = AlignItems::center;
    m_file_path->m_gap = 2.0f;
    m_file_path->m_vars.borderRadius.setAll(4.0f);
    m_file_path->m_context->fitContentFixed = true;
    m_file_path->m_context->statelessElement = true;
    m_file_path->m_context->dontComputeSizeOfChildren = true;
    m_file_path->m_context->overflow[0] = UIOverflowMode::SCROLL;
    m_file_path->m_context->scrollbarWidth[0] = 4.0f;
    m_file_path->m_context->scrollbarBorderRadius = 4.0f;
    m_file_path->m_context->autoScroll = true;
    m_file_path->m_vars.scrollBackgroundColor.setAll(Color(226,226,226));
    m_file_path->m_vars.scrollColor.setAll(Color(200,200,200));
    m_file_path->m_vars.scrollColor.setState(UIElementState::HOVER, Color(180,180,180));
    m_file_path->m_vars.scrollColor.setState(UIElementState::ACTIVE, Color(160,160,160));
    m_file_path->m_context->scrollPosX.setDuration(0.1f);
    m_file_path->m_context->scrollPosX.setTimingFunction(TimingFunctions::easeOutQuad);

    UIElement* new_file_button = new UIElement();
    m_ui->addElement(new_file_button, m_top_bar, m_top_bar, "new_file_button", {});
    new_file_button->m_vars.size.relative.setAll({0.0f, 1.0f});
    new_file_button->m_vars.size.absolute.setAll({44.0f, 0.0f});
    new_file_button->m_vars.borderRadius.setAll(4.0f);
    new_file_button->m_vars.scale.setState(UIElementState::ACTIVE, 0.925f);
    new_file_button->m_vars.scale.setDuration(0.05f);
    new_file_button->m_vars.size.absolute.setAll({44.0f, 0.0f});
    Color bluish(0,128,255);
    new_file_button->m_vars.color.setAll(bluish);
    new_file_button->m_vars.color.setState(UIElementState::HOVER, Color(bluish.h, bluish.s, bluish.l + 0.1f, bluish.a));
    new_file_button->m_vars.color.setState(UIElementState::ACTIVE, Color(bluish.h, bluish.s, bluish.l + 0.2f, bluish.a));
    sf::String new_file_string;
    new_file_string += static_cast<sf::Uint32>(0x1F7A6);
    TextElement *new_file_button_text = new TextElement(new_file_string, conf::fonts::emojis);
    m_ui->addElement(new_file_button_text, new_file_button);
    new_file_button_text->m_vars.origin.relative.setAll({0.5f, 0.5f});
    new_file_button_text->m_vars.pos.relative.setAll({0.5f, 0.5f});
    new_file_button_text->m_vars.color.setAll(Color(255,255,255));

    UIElement* settings_button = new UIElement();
    m_ui->addElement(settings_button, m_top_bar, m_top_bar, "settings_button", {});
    settings_button->m_vars.size.relative.setAll({0.0f, 1.0f});
    settings_button->m_vars.size.absolute.setAll({44.0f, 0.0f});
    settings_button->m_vars.borderRadius.setAll(4.0f);
    settings_button->m_vars.scale.setState(UIElementState::ACTIVE, 0.925f);
    settings_button->m_vars.scale.setDuration(0.05f);
    Color greenish(0, 191, 160);
    greenish.l -= 0.1f;
    settings_button->m_vars.color.setAll(greenish);
    settings_button->m_vars.color.setState(UIElementState::HOVER, Color(greenish.h, greenish.s, greenish.l + 0.03f, greenish.a));
    settings_button->m_vars.color.setState(UIElementState::ACTIVE, Color(greenish.h, greenish.s, greenish.l + 0.06f, greenish.a));
    sf::String settings_string;
    settings_string += static_cast<sf::Uint32>(0x1F6E0);
    TextElement *settings_button_text = new TextElement(settings_string, conf::fonts::emojis);
    m_ui->addElement(settings_button_text, settings_button);
    settings_button_text->m_vars.origin.relative.setAll({0.5f, 0.5f});
    settings_button_text->m_vars.pos.relative.setAll({0.5f, 0.5f});
    settings_button_text->m_vars.color.setAll(Color(255,255,255));

    m_search_bar = new Flex();
    m_ui->addElement(m_search_bar, m_top_bar, m_top_bar, "search_bar", {});
    m_search_bar->m_vars.size.relative.setAll({0.0f, 1.0f});
    m_search_bar->m_vars.size.absolute.setAll({300.0f, 0.0f});
    m_search_bar->m_vars.color.setAll(Color(245,245,245));
    m_search_bar->m_vars.borderColor.setAll(Color(200,200,200));
    m_search_bar->m_vars.borderWidth.setAll(1.0f);
    m_search_bar->m_vars.padding.setAll(3.0f);
    m_search_bar->m_context->sizeMode[1] = UISizeMode::FIXED;
    m_search_bar->m_vars.borderRadius.setAll(4.0f);
    m_search_bar->m_context->overflow[0] = UIOverflowMode::HIDDEN;
    m_search_bar->m_direction = FlexDirection::horizontal;
    m_search_bar->m_justify = JustifyContent::start;
    m_search_bar->m_align = AlignItems::start;
    m_search_bar->m_gap = 3.0f;
    m_search_bar->m_context->fitContentFixed = true;
    m_search_bar->m_context->statelessElement = true;
    m_search_bar->m_context->dontUpdateYourself = true;

    m_search_input = new TextInput("", "", "Search", "Search", conf::fonts::mono_r, 20, 0);
    m_ui->addElement(m_search_input, m_search_bar, m_search_bar, "search_bar_input", {});
    m_search_input->m_vars.size.relative.setAll({0.0f, 1.0f});
    m_search_input->m_vars.flex.setAll(1.0f);
    m_search_input->m_vars.padding.setAll(4.0f);
    m_search_input->m_vars.color.setAll(Color(245,245,245));
    m_search_input->m_context->sizeMode[1] = UISizeMode::FIXED;
    m_search_input->m_context->overflow[0] = UIOverflowMode::SCROLL;
    m_search_input->m_context->autoScroll = true;
    m_search_input->m_context->scrollbarWidth[0] = 4.0f;
    m_search_input->m_context->scrollbarBorderRadius = 0.0f;
    m_search_input->m_vars.scrollBackgroundColor.setAll(Color(200,200,200));
    m_search_input->m_vars.scrollColor.setAll(Color(150,150,150));
    m_search_input->m_vars.scrollColor.setState(UIElementState::HOVER, Color(140,140,140));
    m_search_input->m_vars.scrollColor.setState(UIElementState::ACTIVE, Color(120,120,120));
    m_search_input->m_context->scrollPosX.setDuration(0.1f);
    m_search_input->m_context->scrollPosX.setTimingFunction(TimingFunctions::easeOutQuad);

    m_search_button = new UIElement();
    m_ui->addElement(m_search_button, m_search_bar, m_search_bar, "search_bar_button", {});
    m_search_button->m_vars.size.relative.setAll({0.0f, 1.0f});
    m_search_button->m_vars.size.absolute.setAll({36.0f, 0.0f});
    m_search_button->m_vars.borderRadius.setAll(3.0f);
    Color blue(0, 128, 255);
    m_search_button->m_vars.color.setAll(blue);
    m_search_button->m_vars.color.setState(UIElementState::HOVER, Color(blue.h, blue.s, blue.l + 0.1f, blue.a));
    m_search_button->m_vars.color.setState(UIElementState::ACTIVE, Color(blue.h, blue.s, blue.l + 0.2f, blue.a));
    sf::String s;
    s += static_cast<sf::Uint32>(0x1F50D);
    TextElement *m_search_button_text = new TextElement(s, conf::fonts::emojis);
    m_ui->addElement(m_search_button_text, m_search_button);
    m_search_button_text->m_vars.origin.relative.setAll({0.5f, 0.5f});
    m_search_button_text->m_vars.pos.relative.setAll({0.5f, 0.5f});

    m_search_button->m_context->onClick.push_back([this](MouseContext& m) {
        sf::String input = m_search_input->m_text->m_string;
        std::filesystem::path p(input.toWideString());
        m_model.search(p);
    });
    m_search_input->m_context->onPressKeys.push_back(std::make_pair(
        ui::Keypress(sf::Keyboard::Enter, false, false, false, false),
        [this]() {
            m_search_button->m_context->triggerEvents(m_search_button->m_context->onClick, MouseContext());
        }
    ));

    fileFields = {"Name", "Size", "Last Change"};

    Flex* sort_row = new Flex();
    m_ui->addElement(sort_row, m_layout, m_layout, "sort_bar", {});
    sort_row->m_vars.size.relative.setAll({1.0f, 0.0f});
    sort_row->m_vars.size.absolute.setAll({0.0f, 36.0f});
    sort_row->m_vars.color.setAll(Color(200,200,200));
    sort_row->m_direction = FlexDirection::horizontal;
    sort_row->m_justify = JustifyContent::space_between;
    sort_row->m_gap = 1.0f;
    sort_row->m_context->dontUpdateYourself = true;
    sort_row->m_context->statelessElement = true;

    sort_row->m_vars.shadowBlurLength.setAll(10.0f);
    sort_row->m_vars.shadowColorStart.setAll(Color(0.0f,0.0f,0.0f,0.1f));
    sort_row->m_vars.shadowColorEnd.setAll(Color(0.0f,0.0f,0.0f,0.0f));

    fileFieldsFlexSize.resize((int)FileField::FIELD_AMOUNT);
    for (int i = 0; i < (int)FileField::FIELD_AMOUNT; i++) {
        fileFieldsFlexSize[i] = std::make_pair(i == 0 ? 1.0f : 0.0f, i == 0 ? 0.0f : 175.0f);
    }

    std::vector<int*> sort_choice_states(3, nullptr);
    for (int i = 0; i < sort_choice_states.size(); i++)
        sort_choice_states[i] = new int(0);
    sf::String right_arrow;
    right_arrow += static_cast<sf::Uint32>(0x1F872);
    for (int i = 0; i < (int)FileField::FIELD_AMOUNT; i++) {
        UIElement* sort_choice = new UIElement();
        m_ui->addElement(sort_choice, sort_row, sort_row, "sort_choice_" + std::to_string(i), {});
        sort_choice->m_vars.size.relative.setAll({0.0f, 1.0f});
        sort_choice->m_vars.flex.setAll(fileFieldsFlexSize[i].first);
        sort_choice->m_vars.size.absolute.setAll({fileFieldsFlexSize[i].second, 0.0f});
        sort_choice->m_vars.color.setAll(Color(245,245,245));
        sort_choice->m_vars.color.setState(UIElementState::HOVER, Color(207,227,247));
        sort_choice->m_vars.color.setState(UIElementState::ACTIVE, Color(178,213,248));
        sort_choice->m_context->fitContentFixed = true;

        TextElement* sort_choice_text = new TextElement(fileFields[i], conf::fonts::mono_r, 20);
        m_ui->addElement(sort_choice_text, sort_choice);
        sort_choice_text->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
        sort_choice_text->m_vars.color.setAll(Color(40,40,40));
        sort_choice_text->m_vars.origin.relative.setAll({0.0f, 0.5f});
        sort_choice_text->m_vars.pos.relative.setAll({0.0f, 0.5f});
        sort_choice_text->m_vars.pos.absolute.setAll({10.0f, 0.0f});

        TextElement* sort_choice_arrow = new TextElement(right_arrow, conf::fonts::emojis, 15);
        m_ui->addElement(sort_choice_arrow, sort_choice);
        sort_choice_arrow->m_vars.color.setAll(Color(40,40,40));
        sort_choice_arrow->m_vars.origin.relative.setAll({1.0f, 0.5f});
        sort_choice_arrow->m_vars.pos.relative.setAll({1.0f, 0.5f});
        sort_choice_arrow->m_vars.pos.absolute.setAll({-10.0f, 0.0f});
        sort_choice_arrow->m_vars.opacity.setAll(0.0f);
        sort_choice_arrow->m_vars.opacity.setDuration(0.2f);
        sort_choice_arrow->m_vars.rotate.setDuration(0.2f);

        sort_choice->m_context->onClick.push_back([this, i, sort_choice_states, sort_row, sort_choice_arrow](MouseContext& m) {
            if (*sort_choice_states[i] == 0) {
                m_model.sortEntries((FileField)i, true);
                *sort_choice_states[i] = 1;
            }
            else if (*sort_choice_states[i] == 1) {
                m_model.sortEntries((FileField)i, false);
                *sort_choice_states[i] = 2;
            }
            else if (*sort_choice_states[i] == 2) {
                m_model.sortEntries(FileField::FIELD_AMOUNT, true);
                *sort_choice_states[i] = 0;
            }

            for (int j = 0; j < (int)FileField::FIELD_AMOUNT; j++) {
                if (i == j && *sort_choice_states[j] != 0) {
                    sort_row->m_children[j]->m_children[1]->m_vars.opacity.setAllSmoothly(1.0f);
                }
                else {
                    sort_row->m_children[j]->m_children[1]->m_vars.opacity.setAllSmoothly(0.0f);
                }

                if (i != j)
                    *sort_choice_states[j] = 0;
            }

            sort_row->m_children[i]->m_children[1]->m_vars.rotate.setAllSmoothly(
                *sort_choice_states[i] == 1 ? -90.0f : 90.0f);
        });
    }

    UIElement* padding_button = new UIElement();
    m_ui->addElement(padding_button, sort_row);
    padding_button->m_vars.size.relative.setAll({0.0f, 1.0f});
    padding_button->m_vars.size.absolute.setAll({16.0f, 0.0f});
    Color green = blue;
    padding_button->m_vars.color.setAll(green);
    padding_button->m_vars.color.setState(UIElementState::HOVER, Color(green.h, green.s, green.l + 0.1f, green.a));
    padding_button->m_vars.color.setState(UIElementState::ACTIVE, Color(green.h, green.s, green.l + 0.2f, green.a));

    m_file_window = new Flex();
    m_ui->addElement(m_file_window, m_layout, m_layout, "file_window", {});
    m_file_window->m_vars.size.relative.setAll({1.0f, 0.0f});
    m_file_window->m_vars.flex.setAll(1.0f);
    m_file_window->m_vars.color.setAll(Color(245,245,245));
    m_file_window->m_context->overflow[1] = UIOverflowMode::SCROLL;
    m_file_window->m_context->scrollbarWidth[1] = 16.0f;
    m_file_window->m_context->scrollbarBorderRadius = 0.0f;
    m_file_window->m_vars.scrollBackgroundColor.setAll(Color(200,200,200));
    m_file_window->m_vars.scrollColor.setAll(Color(150,150,150));
    m_file_window->m_vars.scrollColor.setState(UIElementState::HOVER, Color(140,140,140));
    m_file_window->m_vars.scrollColor.setState(UIElementState::ACTIVE, Color(120,120,120));
    m_file_window->m_context->scrollPosY.setTimingFunction(TimingFunctions::easeOutQuad);

    m_file_window->m_direction = FlexDirection::vertical;
    m_file_window->m_context->copyTransformToChildren = true;
    m_file_window->m_context->transformFixed = true;
    m_file_window->m_context->fitContentFixed = true;
    m_file_window->m_context->dontComputeOutOfBoundsChildren = true;
    m_file_window->m_context->scrollWheelDistance = conf::settings::scrollDist;
    //m_file_window->m_context->addChildrenWithSmallZIndexes = true;
    //m_file_window->m_context->propogateHover = true;
    //for (int i = 0; i < (int)sf::Mouse::ButtonCount; i++) {
    //    m_file_window->m_context->propogateMouse[i] = true;
    //}

    // A hack so that file button texts get repositioned after a window size change.
    // If we don't do this we would have to recompute m_file_window's flex positions
    // on every scroll position change, which is way too slow.
    m_ui->m_afterDirtyCheckCalls.push_back([this]() {
        if (m_win->m_context->dirtyBounds) {
            m_win->propogateCall([](UIElement* e){
                e->m_context->contentSizeComputed = 0;
                return true;
            });
        }
    });

    m_ui->m_globalError = new Flex();
    m_ui->addElement(m_ui->m_globalError, m_win, m_win, "error", {});
    m_ui->m_globalError->m_vars.size.absolute.setAll({300.0f, 0.0f});
    m_ui->m_globalError->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    m_ui->m_globalError->m_vars.origin.relative.setAll({0.0f, 1.0f});
    m_ui->m_globalError->m_vars.pos.relative.setAll({0.0f, 1.0f});
    m_ui->m_globalError->m_vars.pos.absolute.setAll({16.0f, -16.0f});
    m_ui->m_globalError->m_vars.color.setAll(Color(255,80,80));
    m_ui->m_globalError->m_vars.opacity.setState(UIElementState::DISABLED, 0.0f);
    m_ui->m_globalError->m_vars.opacity.setDuration(0.2f);
    m_ui->m_globalError->m_vars.padding.setAll(16.0f);
    m_ui->m_globalError->m_vars.borderRadius.setAll(10.0f);
    m_ui->m_globalError->m_context->z_index = 30;
    m_ui->m_globalError->setState(UIElementState::DISABLED);
    m_ui->m_globalError->m_context->dontUpdateChildren = true;
    m_ui->m_globalError->m_context->statelessChildren = true;

    size_t error_filechange_id = m_ui->addError("Error renaming, deleting or creating the file.     Might need to turn on the Administrator mode.");
    size_t error_duplicate_name = m_ui->addError("Error renaming/creating a file. Duplicate name.");

    m_file_options = new Flex();
    m_ui->addElement(m_file_options, m_win, m_win, "file_options", {});
    m_file_options->m_direction = FlexDirection::vertical;
    m_file_options->m_vars.size.absolute.setAll({130.0f, 0.0f});
    m_file_options->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    m_file_options->m_vars.color.setAll(Color(255,255,255));
    m_file_options->m_vars.borderColor.setAll(Color(200,200,200));
    m_file_options->m_vars.shadowBlurLength.setAll(5.0f);
    //m_file_options->m_vars.shadowOffset.setAll({0.0f, 1.0f});
    //m_file_options->m_vars.shadowColorStart.setAll(Color(0.0f,0.0f,0.0f,0.2f));
    //m_file_options->m_vars.shadowColorEnd.setAll(Color(245,245,245,0));
    m_file_options->m_vars.borderWidth.setAll(1.0f);
    m_file_options->m_vars.borderRadius.setAll(6.0f);
    m_file_options->m_vars.padding.setAll(3.0f);
    m_file_options->m_vars.opacity.setAll(0.0f);
    m_file_options->m_vars.opacity.setState(UIElementState::FOCUSED, 1.0f);
    m_file_options->m_vars.translate.absolute.setAll({0.0f, 20.0f});
    m_file_options->m_vars.translate.absolute.setState(UIElementState::FOCUSED, {0.0f, 0.0f});
    m_file_options->m_context->focusable = true;
    m_file_options->m_context->onUnfocus.push_back([this]() {
        m_file_options->setState(UIElementState::DISABLED);
    });
    m_file_options->setState(UIElementState::DISABLED);
    m_file_options->m_context->z_index = 50;
    m_file_options->m_vars.translate.absolute.setDuration(0.1f);
    m_file_options->m_vars.opacity.setDuration(0.1f);
    m_file_options->m_vars.pos.absolute.setDuration(0.1f);
    m_file_options->m_vars.pos.absolute.setTimingFunction(TimingFunctions::easeOutQuad);
    m_file_options->m_vars.opacity.setTimingFunction(TimingFunctions::easeOutQuad);
    m_file_options->m_vars.translate.absolute.setTimingFunction(TimingFunctions::easeOutQuad);
    m_file_window->m_context->onScrollChange.push_back([this]() {
        if (m_file_options->m_context->focused) {
            m_file_options->m_context->focused = false;
            m_file_options->setState(UIElementState::DISABLED);
        }
    });

    std::vector<std::string> file_option_strings = {"Rename", "Delete"};

    for (int i = 0; i < file_option_strings.size(); i++) {
        UIElement* file_option = new UIElement();
        m_ui->addElement(file_option, m_file_options, m_file_options, "file_options_" + file_option_strings[i], {});
        file_option->m_vars.size.absolute.setAll({0.0f, 40.0f});
        file_option->m_vars.size.relative.setAll({1.0f, 0.0f});
        file_option->m_vars.borderRadius.setAll(3.0f);
        file_option->m_vars.color.setAll(Color(0,128,255,0));
        file_option->m_vars.color.setState(UIElementState::HOVER, Color(207,227,247));
        file_option->m_vars.color.setState(UIElementState::ACTIVE, Color(178,213,248));
        file_option->m_context->onClick = {[](MouseContext&){}};

        TextElement* file_option_text = new TextElement(file_option_strings[i], conf::fonts::mono_r, 20);
        m_ui->addElement(file_option_text, file_option);
        file_option_text->m_vars.color.setAll(Color(30,30,30));
        file_option_text->m_vars.origin.relative.setAll({0.5f, 0.5f});
        file_option_text->m_vars.pos.relative.setAll({0.5f, 0.5f});
    }
    m_file_option_rename = m_file_options->m_children[0];
    m_file_option_delete = m_file_options->m_children[1];

    m_file_option_delete->m_context->onClick.push_back([this, error_filechange_id](MouseContext& m) {
        OperationResult res = m_model.remove(m_file_options->m_context->data_id);
        m_file_options->m_context->focused = false;
        m_file_options->setState(UIElementState::DISABLED);
        if (res == OperationResult::Failure) {
            m_ui->showError(error_filechange_id, 5000);
        }
    });

    auto modal_class = [](UIElement* e) {
        e->m_vars.opacity.setState(UIElementState::DISABLED, 0.0f);
        e->m_vars.translate.absolute.setState(UIElementState::DISABLED, {0.0f, 50.0f});
        e->m_vars.scale.setState(UIElementState::DISABLED, 0.6f);
        e->setState(UIElementState::DISABLED);
        e->m_vars.opacity.setDuration(0.2f);
        e->m_vars.translate.absolute.setDuration(0.2f);
        e->m_vars.translate.absolute.setTimingFunction(TimingFunctions::easeOutQuad);
        e->m_vars.scale.setDuration(0.2f);
        e->m_vars.scale.setTimingFunction(TimingFunctions::easeOutQuad);
    };
    m_ui->registerClass("modal", modal_class);

    Flex* file_rename_modal = new Flex();
    m_ui->addElement(file_rename_modal, m_ui->m_modalBackdrop, m_ui->m_modalBackdrop, "file_rename_modal", {});
    file_rename_modal->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    file_rename_modal->m_direction = FlexDirection::vertical;
    file_rename_modal->m_startGap = 16.0f;
    file_rename_modal->m_gap = 16.0f;
    file_rename_modal->m_vars.size.absolute.setAll({400.0f, 0.0f});
    file_rename_modal->m_vars.pos.relative.setAll({0.5f, 0.5f});
    file_rename_modal->m_vars.origin.relative.setAll({0.5f, 0.5f});
    file_rename_modal->m_vars.color.setAll(Color(245,245,245));
    file_rename_modal->m_vars.borderRadius.setAll(10.0f);
    file_rename_modal->m_vars.padding.setAll(16.0f);
    m_ui->applyClass(file_rename_modal, "modal");
    file_rename_modal->m_context->fitContentFixed = true;
    file_rename_modal->m_context->statelessElement = true;
    file_rename_modal->m_context->ignoreParentsTranform = true;

    m_file_option_rename->m_context->onClick.push_back([this, file_rename_modal](MouseContext& m) {
        m_file_rename_input->setString(std::any_cast<std::filesystem::path>(m_file_options->m_context->data_any).wstring());
        m_ui->openModal(file_rename_modal);
        m_file_options->m_context->focused = false;
        m_file_options->setState(UIElementState::DISABLED);
    });

    UIElement* file_rename_modal_title = new UIElement();
    m_ui->addElement(file_rename_modal_title, file_rename_modal, file_rename_modal, "file_rename_modal_title", {});
    file_rename_modal_title->m_context->sizeMode[0] = UISizeMode::FIT_CONTENT;
    file_rename_modal_title->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    file_rename_modal_title->m_vars.padding.setAll(10.0f);
    file_rename_modal_title->m_vars.pos.relative.setAll({0.5f, 0.0f});
    file_rename_modal_title->m_vars.origin.relative.setAll({0.5f, 0.5f});
    file_rename_modal_title->m_vars.color.setAll(Color(30,30,30));
    file_rename_modal_title->m_vars.borderRadius.setAll(10.0f);
    file_rename_modal_title->m_context->dontCountTowardsLayout = true;
    file_rename_modal_title->m_context->includeBorderPadding = true;
    file_rename_modal_title->m_context->fitContentFixed = true;
    file_rename_modal_title->m_context->statelessElement = true;
    file_rename_modal_title->m_context->statelessChildren = true;
    file_rename_modal_title->m_context->dontUpdateChildren = true;
    TextElement *file_rename_modal_title_text = new TextElement("Rename", conf::fonts::mono_r_semibold, 26);
    m_ui->addElement(file_rename_modal_title_text, file_rename_modal_title);
    file_rename_modal_title_text->m_vars.color.setAll(Color(245,245,245));

    std::u32string allowedCharsForRenameFile = U"qwwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNMйцукенгшщзхъфывапролджэячсмиитьбюЙЦУКЕНГШЩЗХЪФЫВАПРОЛЛДЖЭЯЧСМИТЬБЮ, .|12345678990_-+()";
    m_file_rename_input = new TextInput("", sf::String::fromUtf32(allowedCharsForRenameFile.begin(), allowedCharsForRenameFile.end()), "New Name", "New Name", conf::fonts::mono_r, 22, 0);
    m_ui->addElement(m_file_rename_input, file_rename_modal, file_rename_modal, "file_rename_input", {});
    m_file_rename_input->m_vars.size.relative.setAll({1.0f, 0.0f});
    m_file_rename_input->m_vars.size.absolute.setAll({0.0f, 50.0f});
    m_file_rename_input->m_vars.padding.setAll(8.0f);
    m_file_rename_input->m_vars.color.setAll(Color(245,245,245));
    m_file_rename_input->m_vars.borderColor.setAll(Color(200,200,200));
    m_file_rename_input->m_vars.borderWidth.setAll(1.0f);
    m_file_rename_input->m_vars.borderRadius.setAll(6.0f);
    m_file_rename_input->m_context->sizeMode[1] = UISizeMode::FIXED;
    m_file_rename_input->m_context->overflow[0] = UIOverflowMode::SCROLL;
    m_file_rename_input->m_context->autoScroll = true;
    m_file_rename_input->m_context->scrollbarWidth[0] = 4.0f;
    m_file_rename_input->m_context->scrollbarBorderRadius = 0.0f;
    m_file_rename_input->m_vars.scrollBackgroundColor.setAll(Color(200,200,200));
    m_file_rename_input->m_vars.scrollColor.setAll(Color(150,150,150));
    m_file_rename_input->m_vars.scrollColor.setState(UIElementState::HOVER, Color(140,140,140));
    m_file_rename_input->m_vars.scrollColor.setState(UIElementState::ACTIVE, Color(120,120,120));
    m_file_rename_input->m_context->scrollPosX.setDuration(0.1f);
    m_file_rename_input->m_context->scrollPosX.setTimingFunction(TimingFunctions::easeOutQuad);

    Flex* settings_modal = new Flex();
    m_ui->addElement(settings_modal, m_ui->m_modalBackdrop, m_ui->m_modalBackdrop, "settings_modal", {});
    settings_modal->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    settings_modal->m_direction = FlexDirection::vertical;
    settings_modal->m_startGap = 20.0f;
    settings_modal->m_gap = 16.0f;
    settings_modal->m_vars.size.absolute.setAll({400.0f, 0.0f});
    settings_modal->m_vars.pos.relative.setAll({0.5f, 0.5f});
    settings_modal->m_vars.origin.relative.setAll({0.5f, 0.5f});
    settings_modal->m_vars.color.setAll(Color(245,245,245));
    settings_modal->m_vars.borderRadius.setAll(10.0f);
    settings_modal->m_vars.padding.setAll(16.0f);
    m_ui->applyClass(settings_modal, "modal");
    settings_modal->m_context->fitContentFixed = true;
    settings_modal->m_context->statelessElement = true;
    settings_modal->m_context->ignoreParentsTranform = true;

    settings_button->m_context->onClick.push_back([this, settings_modal](MouseContext& m) {
        if (m_ui->isModalOpen(settings_modal))
            m_ui->closeModal();
        else
            m_ui->openModal(settings_modal);
    });

    UIElement* settings_modal_title = new UIElement();
    m_ui->addElement(settings_modal_title, settings_modal, settings_modal, "settings_modal_title", {});
    settings_modal_title->m_context->sizeMode[0] = UISizeMode::FIT_CONTENT;
    settings_modal_title->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    settings_modal_title->m_vars.padding.setAll(10.0f);
    settings_modal_title->m_vars.pos.relative.setAll({0.5f, 0.0f});
    settings_modal_title->m_vars.origin.relative.setAll({0.5f, 0.5f});
    settings_modal_title->m_vars.color.setAll(Color(30,30,30));
    settings_modal_title->m_vars.borderRadius.setAll(10.0f);
    settings_modal_title->m_context->dontCountTowardsLayout = true;
    settings_modal_title->m_context->includeBorderPadding = true;
    settings_modal_title->m_context->fitContentFixed = true;
    settings_modal_title->m_context->statelessElement = true;
    settings_modal_title->m_context->statelessChildren = true;
    settings_modal_title->m_context->dontUpdateChildren = true;
    TextElement *settings_modal_title_text = new TextElement("Settings", conf::fonts::mono_r_semibold, 26);
    m_ui->addElement(settings_modal_title_text, settings_modal_title);
    settings_modal_title_text->m_vars.color.setAll(Color(245,245,245));

    auto create_setting_flex_name = [this](Flex* parent, const std::string& name) {
        Flex* flex = new Flex();
        m_ui->addElement(flex, parent, parent, "settings_modal_setting_" + name, {});
        flex->m_justify = JustifyContent::space_between;
        flex->m_align = AlignItems::center;
        flex->m_vars.size.relative.setAll({1.0f, 0.0f});
        flex->m_vars.size.absolute.setAll({0.0f, 22.0f});
        flex->m_vars.color.setAll(Color(0,0,0,0));
        flex->m_vars.borderColor.setAll(Color(0,0,0,0));
        flex->m_context->fitContentFixed = true;
        flex->m_context->dontUpdateYourself = true;
        flex->m_context->statelessElement = true;

        TextElement* text = new TextElement(name, conf::fonts::mono_r, 21);
        m_ui->addElement(text, flex);
        text->m_vars.color.setAll(Color(30,30,30));
        text->m_context->dontUpdateYourself = true;
        text->m_context->statelessElement = true;

        return flex;
    };

    auto create_setting_slider = [this, create_setting_flex_name](Flex* parent, const std::string& name, float start, float min, float max, bool isInt) {
        Flex* flex = create_setting_flex_name(parent, name);

        Flex* flex_right = new Flex();
        m_ui->addElement(flex_right, flex);
        flex_right->m_align = AlignItems::center;
        flex_right->m_justify = JustifyContent::start;
        flex_right->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
        flex_right->m_vars.size.relative.setAll({0.5f, 0.0f});
        flex_right->m_vars.color.setAll(Color(0,0,0,0));
        flex_right->m_vars.borderColor.setAll(Color(0,0,0,0));
        flex_right->m_gap = 8.0f;
        flex_right->m_context->fitContentFixed = true;
        flex_right->m_context->dontUpdateYourself = true;
        flex_right->m_context->statelessElement = true;

        UIElement* text_container = new UIElement();
        m_ui->addElement(text_container, flex_right);
        text_container->m_vars.borderRadius.setAll(4.0f);
        text_container->m_vars.padding.setAll(4.0f);
        text_container->m_vars.color.setAll(Color(245,245,245));
        text_container->m_vars.borderColor.setAll(Color(200,200,206));
        text_container->m_vars.borderWidth.setAll(1.0f);
        text_container->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
        text_container->m_vars.flex.setAll(0.25f);
        text_container->m_context->fitContentFixed = true;
        text_container->m_context->dontUpdateYourself = true;
        text_container->m_context->dontUpdateChildren = true;
        text_container->m_context->statelessElement = true;
        text_container->m_context->statelessChildren = true;

        TextElement* text_container_text = new TextElement(
            isInt ? std::to_string(int(start)) : std::to_string(start).substr(0,4),
            conf::fonts::mono_r_semibold, 16, 0);
        m_ui->addElement(text_container_text, text_container);
        text_container_text->m_vars.origin.relative.setAll({0.5f, 0.0f});
        text_container_text->m_vars.pos.relative.setAll({0.5f, 0.0f});
        text_container_text->m_vars.color.setAll(Color(11,11,11));
        text_container_text->setFixedHeight("0123456789.");

        Slider* s = new Slider(start, min, max, flex_right->m_context->z_index + 1);
        m_ui->addElement(s, flex_right);
        s->m_sliderLine->m_vars.size.absolute.setAll({0.0f, 13.0f});
        s->m_sliderLine->m_vars.color.setAll(Color(210,210,215));
        if (isInt) {
            s->m_onValueChanged = [s, text_container_text]() {
                text_container_text->setString(std::to_string(int(s->m_var)), false);
            };
        }
        else {
            s->m_onValueChanged = [s, text_container_text]() {
                text_container_text->setString(std::to_string(s->m_var).substr(0,4), false);
            };
        }
        s->m_vars.flex.setAll(0.75f);
        Color fill_color = Color(0, 191, 160);
        fill_color.l -= 0.1f;
        s->m_sliderLine->m_fillColor.setAll(fill_color);
        s->m_sliderLine->m_fillColor.setState(UIElementState::HOVER, Color(fill_color.h, fill_color.s, fill_color.l + 0.05f, fill_color.a));
        s->m_sliderLine->m_fillColor.setState(UIElementState::ACTIVE, Color(fill_color.h, fill_color.s, fill_color.l + 0.1f, fill_color.a));
        s->m_sliderLine->m_fillColor.setDuration(0.085f);
        s->m_sliderLine->m_fillColor.setContext(s->m_context);
        s->m_sliderPoint->m_vars.size.absolute.setAll({25.0f, 25.0f});
        s->m_sliderPoint->m_vars.borderWidth.setAll(8.0f);
        s->m_sliderPoint->m_vars.borderColor.setAll(fill_color);
        s->m_sliderPoint->m_vars.borderColor.setState(UIElementState::HOVER, Color(fill_color.h, fill_color.s, fill_color.l + 0.05f, fill_color.a));
        s->m_sliderPoint->m_vars.borderColor.setState(UIElementState::ACTIVE, Color(fill_color.h, fill_color.s, fill_color.l + 0.1f, fill_color.a));
        s->m_sliderPoint->m_vars.borderColor.setDuration(0.085f);
        s->m_sliderPoint->m_vars.borderColor.setContext(s->m_context);
        s->m_sliderPoint->m_vars.scale.setState(UIElementState::HOVER, 1.15f);
        s->m_sliderPoint->m_vars.scale.setState(UIElementState::ACTIVE, 1.3f);
        s->m_sliderPoint->m_vars.scale.setDuration(0.085f);
        s->m_sliderPoint->m_vars.scale.setContext(s->m_context);
        s->m_sliderPoint->m_vars.color.setAll(Color(255,255,255));
        s->m_sliderPoint->m_vars.shadowBlurLength.setContext(s->m_context);
        s->m_sliderPoint->m_vars.shadowBlurLength.setAll(6.0f);
        s->m_sliderPoint->m_vars.shadowColorStart.setAll(Color(0.0f,0.0f,0.0f,0.125f));
        s->m_sliderPoint->m_vars.shadowColorEnd.setAll(Color(245,245,245,0));
        s->m_sliderPoint->m_vars.borderRadius.setAll(100.0f);
        return s;
    };

    auto create_setting_check = [this, create_setting_flex_name](Flex* parent, const std::string& name) {
        Flex* flex = create_setting_flex_name(parent, name);

        Button* check_button = new Button();
        m_ui->addElement(check_button, flex);
        check_button->m_vars.borderRadius.setAll(4.0f);
        check_button->m_vars.size.absolute.setAll({24.0f, 24.0f});
        check_button->m_vars.color.setAll(Color(245,245,245));
        check_button->m_vars.color.setState(UIElementState::HOVER, Color(235,235,235));
        check_button->m_vars.borderColor.setAll(Color(160,160,160));
        check_button->m_vars.borderWidth.setAll(1.0f);
        check_button->m_context->dontUpdateChildren = true;
        check_button->m_context->pressable = true;
        check_button->m_context->fitContentFixed = true;

        sf::String arrow;
        arrow += static_cast<sf::Uint32>(0x2713);
        TextElement* check_button_inside = new TextElement(arrow, conf::fonts::emojis, 20);
        m_ui->addElement(check_button_inside, check_button);
        check_button_inside->m_context->sizeMode[0] = UISizeMode::FIXED;
        check_button_inside->m_context->sizeMode[1] = UISizeMode::FIXED;
        check_button_inside->m_vars.size.relative.setAll({1.0f, 1.0f});
        check_button_inside->m_textOriginRel = {0.5f, 0.5f};
        check_button_inside->m_textPosRel = {0.5f, 0.5f};
        check_button_inside->m_vars.color.setAll(Color(0,128,255));
        check_button_inside->m_context->dontUpdateYourself = true;
        check_button_inside->m_vars.scale.setContext(check_button->m_context);
        check_button_inside->m_vars.scale.setAll(0.0f);
        check_button_inside->m_vars.scale.setState(UIElementState::PRESSED, 1.0f);
        check_button_inside->m_vars.scale.setDuration(0.1f);

        return check_button;
    };

    auto create_modal_cancel_confirm = [this, create_setting_flex_name](Flex* parent, const std::string& modal_name) {
        Flex* flex = new Flex();
        m_ui->addElement(flex, parent);
        flex->m_justify = JustifyContent::start;
        flex->m_align = AlignItems::start;
        flex->m_gap = 16.0f;
        flex->m_vars.size.relative.setAll({1.0f, 0.0f});
        flex->m_vars.size.absolute.setAll({0.0f, 60.0f});
        flex->m_vars.color.setAll(Color(0,0,0,0));
        flex->m_vars.borderColor.setAll(Color(0,0,0,0));
        flex->m_context->fitContentFixed = true;
        flex->m_context->dontUpdateYourself = true;
        flex->m_context->statelessElement = true;

        UIElement* cancel_button = new UIElement();
        m_ui->addElement(cancel_button, flex);
        cancel_button->m_vars.flex.setAll(1.0f);
        cancel_button->m_vars.borderRadius.setAll(6.0f);
        cancel_button->m_vars.size.relative.setAll({0.0, 1.0f});
        cancel_button->m_vars.color.setAll(Color(230,230,230));
        cancel_button->m_vars.color.setState(UIElementState::HOVER, Color(220,220,220));
        cancel_button->m_vars.color.setState(UIElementState::ACTIVE, Color(215,215,215));
        cancel_button->m_vars.scale.setState(UIElementState::ACTIVE, 0.95f);
        cancel_button->m_vars.scale.setTimingFunction(TimingFunctions::easeOutQuad);
        cancel_button->m_vars.scale.setDuration(0.075f);
        cancel_button->m_vars.borderWidth.setAll(0.0f);
        cancel_button->m_context->dontUpdateChildren = true;
        cancel_button->m_context->statelessChildren = true;
        cancel_button->m_context->fitContentFixed = true;
        TextElement* cancel_button_text = new TextElement("Cancel", conf::fonts::mono_r_semibold);
        m_ui->addElement(cancel_button_text, cancel_button, cancel_button, "settings_modal_cancel_text_" + modal_name, {});
        cancel_button_text->m_vars.color.setAll(Color(30,30,30));
        cancel_button_text->m_vars.origin.relative.setAll({0.5f, 0.5f});
        cancel_button_text->m_vars.pos.relative.setAll({0.5f, 0.5f});

        UIElement* confirm_button = new UIElement();
        m_ui->addElement(confirm_button, flex);
        confirm_button->m_vars.flex.setAll(1.0f);
        confirm_button->m_vars.borderRadius.setAll(6.0f);
        confirm_button->m_vars.size.relative.setAll({0.0, 1.0f});
        Color greenish(0, 191, 160);
        greenish.l -= 0.1f;
        confirm_button->m_vars.color.setAll(greenish);
        confirm_button->m_vars.color.setState(UIElementState::HOVER, Color(greenish.h, greenish.s, greenish.l + 0.02f, greenish.a));
        confirm_button->m_vars.color.setState(UIElementState::ACTIVE, Color(greenish.h, greenish.s, greenish.l + 0.04f, greenish.a));
        confirm_button->m_vars.scale.setState(UIElementState::ACTIVE, 0.95f);
        confirm_button->m_vars.scale.setTimingFunction(TimingFunctions::easeOutQuad);
        confirm_button->m_vars.scale.setDuration(0.075f);
        confirm_button->m_vars.borderWidth.setAll(0.0f);
        confirm_button->m_context->dontUpdateChildren = true;
        confirm_button->m_context->statelessChildren = true;
        confirm_button->m_context->fitContentFixed = true;
        TextElement* confirm_button_text = new TextElement("Confirm", conf::fonts::mono_r_semibold);
        m_ui->addElement(confirm_button_text, confirm_button, confirm_button, "settings_modal_confirm_text_" + modal_name, {});
        confirm_button_text->m_vars.origin.relative.setAll({0.5f, 0.5f});
        confirm_button_text->m_vars.pos.relative.setAll({0.5f, 0.5f});

        return std::make_pair(confirm_button, cancel_button);
    };

        Flex* new_file_modal = new Flex();
    m_ui->addElement(new_file_modal, m_ui->m_modalBackdrop, m_ui->m_modalBackdrop, "new_file_modal", {});
    new_file_modal->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    new_file_modal->m_direction = FlexDirection::vertical;
    new_file_modal->m_startGap = 16.0f;
    new_file_modal->m_gap = 16.0f;
    new_file_modal->m_vars.size.absolute.setAll({400.0f, 0.0f});
    new_file_modal->m_vars.pos.relative.setAll({0.5f, 0.5f});
    new_file_modal->m_vars.origin.relative.setAll({0.5f, 0.5f});
    new_file_modal->m_vars.color.setAll(Color(245,245,245));
    new_file_modal->m_vars.borderRadius.setAll(10.0f);
    new_file_modal->m_vars.padding.setAll(16.0f);
    m_ui->applyClass(new_file_modal, "modal");
    new_file_modal->m_context->fitContentFixed = true;
    new_file_modal->m_context->statelessElement = true;
    new_file_modal->m_context->ignoreParentsTranform = true;

    new_file_button->m_context->onClick.push_back([this, new_file_modal](MouseContext& m) {
        m_ui->openModal(new_file_modal);
    });

    UIElement* new_file_modal_title = new UIElement();
    m_ui->addElement(new_file_modal_title, new_file_modal, new_file_modal, "new_file_modal_title", {});
    new_file_modal_title->m_context->sizeMode[0] = UISizeMode::FIT_CONTENT;
    new_file_modal_title->m_context->sizeMode[1] = UISizeMode::FIT_CONTENT;
    new_file_modal_title->m_vars.padding.setAll(10.0f);
    new_file_modal_title->m_vars.pos.relative.setAll({0.5f, 0.0f});
    new_file_modal_title->m_vars.origin.relative.setAll({0.5f, 0.5f});
    new_file_modal_title->m_vars.color.setAll(Color(30,30,30));
    new_file_modal_title->m_vars.borderRadius.setAll(10.0f);
    new_file_modal_title->m_context->dontCountTowardsLayout = true;
    new_file_modal_title->m_context->includeBorderPadding = true;
    new_file_modal_title->m_context->fitContentFixed = true;
    new_file_modal_title->m_context->statelessElement = true;
    new_file_modal_title->m_context->statelessChildren = true;
    new_file_modal_title->m_context->dontUpdateChildren = true;
    TextElement *new_file_modal_title_text = new TextElement("New File", conf::fonts::mono_r_semibold, 26);
    m_ui->addElement(new_file_modal_title_text, new_file_modal_title);
    new_file_modal_title_text->m_vars.color.setAll(Color(245,245,245));
    
    Button* button_IS_DIRECTORY = create_setting_check(new_file_modal, "Directory");

    std::u32string allowedCharsForNewFile = U"qwwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNMйцукенгшщзхъфывапролджэячсмиитьбюЙЦУКЕНГШЩЗХЪФЫВАПРОЛЛДЖЭЯЧСМИТЬБЮ, .|12345678990_-+()";
    m_new_file_input = new TextInput("", sf::String::fromUtf32(allowedCharsForNewFile.begin(), allowedCharsForNewFile.end()), "New File", "Name", conf::fonts::mono_r, 22, 0);
    m_ui->addElement(m_new_file_input, new_file_modal, new_file_modal, "new_file_input", {});
    m_new_file_input->m_vars.size.relative.setAll({1.0f, 0.0f});
    m_new_file_input->m_vars.size.absolute.setAll({0.0f, 50.0f});
    m_new_file_input->m_vars.padding.setAll(8.0f);
    m_new_file_input->m_vars.color.setAll(Color(245,245,245));
    m_new_file_input->m_vars.borderColor.setAll(Color(200,200,200));
    m_new_file_input->m_vars.borderWidth.setAll(1.0f);
    m_new_file_input->m_vars.borderRadius.setAll(6.0f);
    m_new_file_input->m_context->sizeMode[1] = UISizeMode::FIXED;
    m_new_file_input->m_context->overflow[0] = UIOverflowMode::SCROLL;
    m_new_file_input->m_context->autoScroll = true;
    m_new_file_input->m_context->scrollbarWidth[0] = 4.0f;
    m_new_file_input->m_context->scrollbarBorderRadius = 0.0f;
    m_new_file_input->m_vars.scrollBackgroundColor.setAll(Color(200,200,200));
    m_new_file_input->m_vars.scrollColor.setAll(Color(150,150,150));
    m_new_file_input->m_vars.scrollColor.setState(UIElementState::HOVER, Color(140,140,140));
    m_new_file_input->m_vars.scrollColor.setState(UIElementState::ACTIVE, Color(120,120,120));
    m_new_file_input->m_context->scrollPosX.setDuration(0.1f);
    m_new_file_input->m_context->scrollPosX.setTimingFunction(TimingFunctions::easeOutQuad);

    const auto newfile_confirm_cancel = create_modal_cancel_confirm(new_file_modal, "newfile");


    const auto rename_confirm_cancel = create_modal_cancel_confirm(file_rename_modal, "rename");

    Button* button_SMOOTH_SORT = create_setting_check(settings_modal, "Smooth Sort");
    Button* button_SMOOTH_SCROLL = create_setting_check(settings_modal, "Smooth Scroll");
    Slider* slider_FPS = create_setting_slider(settings_modal, "FPS", conf::settings::FPS, 20.0f, 240.0f, true);
    Slider* slider_SCROLL = create_setting_slider(settings_modal, "Scroll Dist", m_file_window->m_context->scrollWheelDistance, 20.0f, 400.0f, true);
    const auto settings_confirm_cancel = create_modal_cancel_confirm(settings_modal, "settings");

    if (conf::settings::smoothScroll) {
        button_SMOOTH_SCROLL->click();
        m_file_window->m_context->scrollPosY.setDuration(0.1f);
    }
    if (conf::settings::smoothSort) {
        button_SMOOTH_SORT->click();
        m_file_window->m_context->dontComputeSizeOfChildren = false;
    }
    else {
        m_file_window->m_context->dontComputeSizeOfChildren = true;
    }

    newfile_confirm_cancel.second->m_context->onClick.push_back([this](MouseContext& m) {
        m_ui->closeModal();
    });
    newfile_confirm_cancel.first->m_context->onClick.push_back([this, error_filechange_id, error_duplicate_name, button_IS_DIRECTORY](MouseContext& m) {
        const bool is_dir = button_IS_DIRECTORY->m_context->pressed;
        const std::filesystem::path p(m_new_file_input->m_text->m_string.toWideString());
        if (p.wstring().empty())
            return;

        OperationResult res = m_model.create(p, is_dir);
        m_ui->closeModal();

        if (res == OperationResult::Failure) {
            m_ui->showError(error_filechange_id, 5000);
        }
        else if (res == OperationResult::TargetAlreadyExists) {
            m_ui->showError(error_duplicate_name, 5000);
        }
    });

    rename_confirm_cancel.second->m_context->onClick.push_back([this](MouseContext& m) {
        m_ui->closeModal();
    });
    rename_confirm_cancel.first->m_context->onClick.push_back([this, error_filechange_id, error_duplicate_name](MouseContext& m) {
        const std::filesystem::path p(m_file_rename_input->m_text->m_string.toWideString());
        if (p.wstring().empty())
            return;

        OperationResult res = m_model.rename(m_file_options->m_context->data_id, p);
        m_ui->closeModal();

        if (res == OperationResult::Failure) {
            m_ui->showError(error_filechange_id, 5000);
        }
        else if (res == OperationResult::TargetAlreadyExists) {
            m_ui->showError(error_duplicate_name, 5000);
        }
    });

    settings_confirm_cancel.second->m_context->onClick.push_back([this](MouseContext& m) {
        m_ui->closeModal();
    });
    settings_confirm_cancel.first->m_context->onClick.push_back(
        [this, button_SMOOTH_SORT, button_SMOOTH_SCROLL, slider_FPS, slider_SCROLL](MouseContext& m) 
    {
        const bool smooth_scroll = button_SMOOTH_SCROLL->m_context->pressed;
        const bool smooth_sort = button_SMOOTH_SORT->m_context->pressed;
        const int FPS = int(slider_FPS->m_var);
        const int SCROLL_STRENGTH = int(slider_SCROLL->m_var);

        if (smooth_scroll != conf::settings::smoothScroll) {
            if (smooth_scroll) {
                m_file_window->m_context->scrollPosY.setDuration(0.1f);
            }
            else {
                m_file_window->m_context->scrollPosY.setDuration(0.0f);
            }
            conf::settings::smoothScroll = smooth_scroll;
        }
        if (smooth_sort != conf::settings::smoothSort) {
            if (smooth_sort) {    
                m_file_window->m_context->dontComputeSizeOfChildren = false;
            }   
            else {
                m_file_window->m_context->dontComputeSizeOfChildren = true;
            }
            conf::settings::smoothSort = smooth_sort;
            
            std::vector<UIElement*>& fileElements = m_fileElementPool.getItems();
            for (int i = 0; i < fileElements.size(); i++) {
                if (conf::settings::smoothSort) {
                    fileElements[i]->m_vars.pos.absolute.setDuration(0.2f);
                }
                else {
                    fileElements[i]->m_vars.pos.absolute.setDuration(0.0f);
                }
            }

            std::vector<UIElement*>& detailedElements = m_detailedFileElementPool.getItems();
            for (int i = 0; i < detailedElements.size(); i++) {
                if (conf::settings::smoothSort) {
                    detailedElements[i]->m_vars.pos.absolute.setDuration(0.2f);
                }
                else {
                    detailedElements[i]->m_vars.pos.absolute.setDuration(0.0f);
                }
            }
        }
        if (FPS != conf::settings::FPS) {
            conf::settings::FPS = FPS;
            m_ui->m_engineContext.m_window->setFramerateLimit(FPS);
        }
        if (SCROLL_STRENGTH != m_file_window->m_context->scrollWheelDistance) {
            conf::settings::scrollDist = SCROLL_STRENGTH;
            m_file_window->m_context->scrollWheelDistance = SCROLL_STRENGTH;
        }
        m_ui->closeModal();
    });

    sync::StateManager::addCallback(sync::State::NORMAL, [this]() {
        m_model.update();
    });

    EventBus::subscribe("refresh", [this, sort_row, sort_choice_states](const std::string& data) {
        // Gets called by model.setPath() and model.remove(), updates the UI after changing the currentPath or removing a file.
        
        // the file window
        const std::vector<File>& entries = m_model.getEntries();

        m_file_window->m_children.clear();
        const std::filesystem::path& currentPath = m_model.getCurrentPath();

        m_fileElementPool.freeAll();
        File new_file(std::filesystem::path("."), std::filesystem::path("."), 0, "", true, 0);
        while (m_fileElementPool.getSize() < entries.size() + 1) {
            m_fileElementPool.add(createFileButton(new_file));
        }

        if (FileSystem::hasParent(currentPath)) {
            std::filesystem::path parent_path = FileSystem::getParentPath(currentPath);
            UIElement* e = m_fileElementPool.yield();
            renameFileButton(e, File(parent_path, std::filesystem::path(".."), 0, "", true, std::filesystem::hash_value(parent_path)));
            m_file_window->addChild(e);
        }
        for (const File& f : entries) {
            UIElement* e = m_fileElementPool.yield();
            renameFileButton(e, f);
            m_file_window->addChild(e);
        }
        for (int i = 0; i < m_file_window->m_children.size(); i++) {
            if (m_file_window->m_children[i]->m_vars.flex_order.get() != i + 1) {
                m_file_window->m_children[i]->m_vars.flex_order.setAll(i + 1);
            }
        }

        // the path bar
        m_pathElementPool.freeAll();
        m_pathDelmiterPool.freeAll();

        m_file_path->m_children.clear();

        const std::vector<std::filesystem::path>& fullpath = m_model.getFullPath();
        while (m_pathElementPool.getSize() < fullpath.size()) {
            m_pathElementPool.add(createPathButton(std::filesystem::path("/"), std::filesystem::path("/")));
        }
        while (m_pathDelmiterPool.getSize() < int(fullpath.size()) - 1) {
            m_pathDelmiterPool.add(createPathDelimeter());
        }
        for (int i = 0; i < fullpath.size(); i++) {
            if (i == 0) {
                UIElement* path = m_pathElementPool.yield();
                #ifdef _WIN32
                    renamePathButton(path, fullpath[i], u"This Computer");
                #else
                    renamePathButton(path, fullpath[i], "root");
                #endif
                m_file_path->addChild(path);
            }
            else if (i == 1) {
                UIElement* path = m_pathElementPool.yield();
                UIElement* deli = m_pathDelmiterPool.yield();
                m_file_path->addChild(deli);
                #ifdef _WIN32
                    renamePathButton(path, fullpath[i], std::filesystem::path(fullpath[i].wstring().substr(0,2)));
                #else
                    renamePathButton(path, fullpath[i], fullpath[i].filename());
                #endif
                m_file_path->addChild(path);
            }
            else {
                UIElement* path = m_pathElementPool.yield();
                UIElement* deli = m_pathDelmiterPool.yield();
                m_file_path->addChild(deli);
                renamePathButton(path, fullpath[i], fullpath[i].filename());
                m_file_path->addChild(path);
            }
        }
        for (int i = 0; i < (int)FileField::FIELD_AMOUNT; i++) {
            sort_row->m_children[i]->m_children[1]->m_vars.opacity.setAllSmoothly(0.0f);
            *sort_choice_states[i] = 0;
        }

        if (data != "no_scroll_reset")
            m_file_window->m_context->scrollPosY.setInstantly(0.0f);
        m_file_window->m_context->contentSizeComputed = 0;
        m_file_window->m_context->transformComputed = 0;
        m_file_window->m_context->dirtyBounds = true;
        m_file_path->m_context->scrollPosX.setInstantly(100000.0f);
        m_file_path->m_context->contentSizeComputed = 0;
        m_file_path->m_context->dirtyBounds = true;
    });

    EventBus::subscribe("rename", [this, sort_row, sort_choice_states](const std::string& data) {
        // Gets called by model.rename(), updates the UI after renaming a file.
        
        const size_t delim_pos = data.find(",");
        const std::string id_before_str = data.substr(0, delim_pos);
        const std::string id_after_str = data.substr(delim_pos + 1);

        size_t id_before = std::stoull(id_before_str);
        size_t id_after = std::stoull(id_after_str);

        size_t entry_idx = 0;
        const std::vector<File>& entries = m_model.getEntries();
        for (int i = 0; i < entries.size(); i++) {
            if (entries[i].id == id_after) {
                entry_idx = i;
                break;
            } 
        }

        const std::vector<UIElement*>& items = m_fileElementPool.getItems();
        for (int i = 0; i < items.size(); i++) {
            if (items[i]->m_context->data_id == id_before) {
                renameFileButton(items[i], entries[entry_idx]);
                break;
            }
        }
    });

    EventBus::subscribe("search_start", [this, sort_row, sort_choice_states](const std::string& data) {
        // Gets called by model.search(), updates the UI after search has been started.

        m_detailedFileElementPool.freeAll();
        m_file_window->m_children.clear();
    });

    EventBus::subscribe("refresh_search", [this, sort_row, sort_choice_states](const std::string& data) {
        // Gets called by model.update(), updates the UI after new search results have been found.

        const std::vector<File>& entries = m_model.getEntries();

        File new_file(std::filesystem::path("."), std::filesystem::path("."), 0, "", true, 0);
        while (m_detailedFileElementPool.getSize() < entries.size()) {
            m_detailedFileElementPool.add(createDetailedFileButton(new_file));
        }

        std::unordered_map<size_t, size_t> id_to_position;
        std::unordered_map<size_t, UIElement*> id_to_element;

        for (size_t i = 0; i < entries.size(); i++) {
            id_to_position[entries[i].id] = i;
        }

        auto new_end_it = std::partition(
            m_file_window->m_children.begin(), 
            m_file_window->m_children.end(), 
            [&id_to_position](UIElement* e) { 
                return id_to_position.find(e->m_context->data_id) != id_to_position.end();
            }
        );
        for (auto it = new_end_it; it != m_file_window->m_children.end(); it++) {
            m_detailedFileElementPool.free(*it);
        }
        m_file_window->m_children.resize(new_end_it - m_file_window->m_children.begin());

        for (size_t i = 0; i < m_file_window->m_children.size(); i++) {
            id_to_element[m_file_window->m_children[i]->m_context->data_id] = m_file_window->m_children[i];
        }

        for (size_t i = 0; i < entries.size(); i++) {
            const auto& it = id_to_element.find(entries[i].id);
            if (it == id_to_element.end()) {
                UIElement *e = m_detailedFileElementPool.yield();
                renameDetailedFileButton(e, entries[i]);
                m_file_window->addChild(e);
                id_to_element[entries[i].id] = e;
            }
        }

        for (size_t i = 0; i < entries.size(); i++) {
            int flex_order = id_to_element[entries[i].id]->m_vars.flex_order.get();
            if (flex_order != (i + 1)) {
                id_to_element[entries[i].id]->m_vars.flex_order.setAll(i + 1);
            }
        }

        for (int i = 0; i < (int)FileField::FIELD_AMOUNT; i++) {
            sort_row->m_children[i]->m_children[1]->m_vars.opacity.setAllSmoothly(0.0f);
            *sort_choice_states[i] = 0;
        }

        m_file_window->m_context->contentSizeComputed = 0;
        m_file_window->m_context->transformComputed = 0;
        m_file_window->m_context->dirtyBounds = true;
    });

    EventBus::subscribe("sort", [this](const std::string& data) {
        // Gets called by model.sortEntries(), updates the UI after a sort has been performed

        std::unordered_map<size_t, UIElement*> name_to_element;

        for (int i = 0; i < m_file_window->m_children.size(); i++) {
            size_t element_id = m_file_window->m_children[i]->m_context->data_id;
            name_to_element[element_id] = m_file_window->m_children[i];
        }

        const std::vector<File>& entries = m_model.getEntries();

        bool name_not_found = false;

        for (int i = 0; i < entries.size(); i++) {
            if (name_to_element.find(entries[i].id) == name_to_element.end()) {
                name_not_found = true;
                break;
            }
            name_to_element[entries[i].id]->m_vars.flex_order.setAll(i + 1);
        }

        if (!name_not_found) {
            m_file_window->m_context->contentSizeComputed = 0;
            m_file_window->m_context->dirtyBounds = true;
        }
    });
}

UIElement* FileExplorerView::createFileButton(const File& f) {
    Flex* file_element = new Flex();
    file_element->m_vars.size.relative.setAll({1.0f, 0.0f});
    file_element->m_vars.size.absolute.setAll({0.0f, 40.0f});
    file_element->m_vars.color.setAll(Color(245,245,245));
    file_element->m_vars.color.setState(UIElementState::HOVER, Color(207,227,247));
    file_element->m_vars.color.setState(UIElementState::ACTIVE, Color(178,213,248));
    file_element->m_context->onClick.push_back([this, f](MouseContext& m) {
        m_model.setPath(f.path);
    });
    file_element->m_context->onPress[1].push_back([this, f, file_element](MouseContext& m) {
        sf::Vector2f pos = sf::Vector2f(m.m_current.pos);
        if (pos.x + m_file_options->m_context->anchorBounds.m_width > m_win->m_context->anchorBounds.m_width) {
            pos.x -= m_file_options->m_context->anchorBounds.m_width;
        }
        if (pos.y + m_file_options->m_context->anchorBounds.m_height > m_win->m_context->anchorBounds.m_height) {
            pos.y -= m_file_options->m_context->anchorBounds.m_height;
        }
        if (m_file_options->m_context->focused)
            m_file_options->m_vars.pos.absolute.setAllSmoothly(pos);
        else 
            m_file_options->m_vars.pos.absolute.setAll(pos);
        m_file_options->m_context->focused = true;
        m_file_options->setState(UIElementState::FOCUSED);

        m_file_options->m_context->data_id = f.id;
        m_file_options->m_context->data_any = f.name;
    });
    file_element->m_align = AlignItems::center;
    file_element->m_vars.pos.absolute.setTimingFunction(TimingFunctions::easeOutQuad);

    file_element->m_context->data_id = f.id;
    file_element->m_context->data_type = (size_t)ButtonType::FILE;
    

    if (conf::settings::smoothSort) {
        file_element->m_vars.pos.absolute.setDuration(0.2f);
    }

    file_element->m_context->copyTransformToChildren = true;
    file_element->m_context->dontUpdateChildren = true;
    file_element->m_context->statelessChildren = true;
    file_element->m_context->fitContentFixed = true;
    file_element->m_context->transformFixed = true;
    file_element->m_context->dontComputeSizeOfChildren = true;

    TextElement* text_file_type = new TextElement(
        f.is_directory ? m_stringDirectory : m_stringFile, 
        f.is_directory ? conf::fonts::emojis : conf::fonts::emoji_type, 24);
    text_file_type->m_vars.size.absolute.setAll({44.0f, 5.0f});
    text_file_type->m_vars.color.setAll(f.is_directory ? Color(255,233,153) : Color(100,100,100));
    text_file_type->m_vars.borderColor.setAll(Color(238,203,74));
    text_file_type->m_vars.borderWidth.setAll(f.is_directory ? 2.0f : 0.0f);
    text_file_type->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_file_type->m_context->sizeMode[1] = UISizeMode::FIXED;
    text_file_type->m_textOriginRel = {0.5f, 0.5f};
    text_file_type->m_textPosRel = {0.5f, 0.5f};
    file_element->addChild(text_file_type);
    text_file_type->m_context->z_index = 10;

    TextElement* text_name = new TextElement(f.name.wstring(), conf::fonts::mono_r, 22);
    text_name->m_vars.flex.setAll(fileFieldsFlexSize[0].first);
    text_name->m_vars.size.absolute.setAll({fileFieldsFlexSize[0].second, 0.0f});
    text_name->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_name->m_vars.color.setAll(Color(40,40,40));
    text_name->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_name->m_textOverflow = TextOverflow::ELLIPSIS;
    text_name->m_textPosAbs = {3.0f, 0.0f};
    file_element->addChild(text_name);
    text_name->m_context->z_index = 10;

    TextElement* text_size = new TextElement(f.is_directory ? "" : util::formatBytes(f.bytes), conf::fonts::mono_r, 22);
    text_size->m_vars.flex.setAll(fileFieldsFlexSize[1].first);
    text_size->m_vars.size.absolute.setAll({fileFieldsFlexSize[1].second, 0.0f});
    text_size->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_size->m_vars.color.setAll(Color(128,128,128));
    text_size->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_size->m_textPosAbs = {-10.0f, 0.0f};
    text_size->m_textPosRel = {1.0f, 0.0f};
    text_size->m_textOriginRel = {1.0f, 0.0f};
    file_element->addChild(text_size);
    text_size->m_context->z_index = 10;

    TextElement* text_date = new TextElement(util::formatDate(f.last_write_time), conf::fonts::mono_r, 22);
    text_date->m_vars.flex.setAll(fileFieldsFlexSize[2].first);
    text_date->m_vars.size.absolute.setAll({fileFieldsFlexSize[2].second, 0.0f});
    text_date->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_date->m_vars.color.setAll(Color(128,128,128));
    text_date->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_date->m_textPosAbs = {-10.0f, 0.0f};
    text_date->m_textPosRel = {1.0f, 0.0f};
    text_date->m_textOriginRel = {1.0f, 0.0f};
    file_element->addChild(text_date);
    text_date->m_context->z_index = 10;

    return file_element;
}

void FileExplorerView::renameFileButton(UIElement* e, const File& f) {
    e->m_context->onClick[0] = [this, f](MouseContext& m) {
        m_model.setPath(f.path);
    };
    if (conf::settings::smoothSort) {
        e->m_vars.pos.absolute.setDuration(0.2f);
    }
    else {
        e->m_vars.pos.absolute.setDuration(0.0f);
    }
    e->m_context->data_id = f.id;

    e->m_context->onPress[1][0] = [this, f](MouseContext& m) {
        sf::Vector2f pos = sf::Vector2f(m.m_current.pos);
        if (pos.x + m_file_options->m_context->anchorBounds.m_width > m_win->m_context->anchorBounds.m_width) {
            pos.x -= m_file_options->m_context->anchorBounds.m_width;
        }
        if (pos.y + m_file_options->m_context->anchorBounds.m_height > m_win->m_context->anchorBounds.m_height) {
            pos.y -= m_file_options->m_context->anchorBounds.m_height;
        }
        if (m_file_options->m_context->focused)
            m_file_options->m_vars.pos.absolute.setAllSmoothly(pos);
        else 
            m_file_options->m_vars.pos.absolute.setAll(pos);
        m_file_options->m_context->focused = true;
        m_file_options->setState(UIElementState::FOCUSED);

        m_file_options->m_context->data_id = f.id;
        m_file_options->m_context->data_any = f.name;
    };

    TextElement* text_type = (TextElement*)e->m_children[0];
    TextElement* text_name = (TextElement*)e->m_children[1];
    TextElement* text_size = (TextElement*)e->m_children[2];
    TextElement* text_date = (TextElement*)e->m_children[3];

    text_type->m_text.setFont(f.is_directory ? conf::fonts::emojis : conf::fonts::emoji_type);
    text_type->m_vars.color.setAll(f.is_directory ? Color(255,233,153) : Color(100,100,100));
    text_type->m_vars.borderWidth.setAll(f.is_directory ? 2.0f : 0.0f);
    text_type->setString(f.is_directory ? m_stringDirectory : m_stringFile, false);
    text_name->setString(f.name.wstring());
    text_size->setString(f.is_directory ? "" : util::formatBytes(f.bytes), false);
    text_date->setString(util::formatDate(f.last_write_time), false);
}

UIElement* FileExplorerView::createDetailedFileButton(const File& f) {
    Flex* file_element = new Flex();
    file_element->m_vars.size.relative.setAll({1.0f, 0.0f});
    file_element->m_vars.size.absolute.setAll({0.0f, 70.0f});
    file_element->m_vars.color.setAll(Color(245,245,245));
    file_element->m_vars.color.setState(UIElementState::HOVER, Color(207,227,247));
    file_element->m_vars.color.setState(UIElementState::ACTIVE, Color(178,213,248));
    if (std::filesystem::is_directory(f.path)) {
        file_element->m_context->onClick.push_back([this, f](MouseContext& m) {
            m_model.setPath(f.path);
        });
    }
    else {
        const std::filesystem::path parent = f.path.parent_path();
        file_element->m_context->onClick.push_back([this, parent](MouseContext& m) {
            m_model.setPath(parent);
        });
    }
    file_element->m_align = AlignItems::center;
    file_element->m_vars.pos.absolute.setTimingFunction(TimingFunctions::easeOutQuad);

    file_element->m_context->data_id = f.id;
    file_element->m_context->data_type = (size_t)ButtonType::FILE_DETAILED;

    if (conf::settings::smoothSort) {
        file_element->m_vars.pos.absolute.setDuration(0.2f);
    }

    file_element->m_context->copyTransformToChildren = true;
    file_element->m_context->dontUpdateChildren = true;
    file_element->m_context->statelessChildren = true;
    file_element->m_context->fitContentFixed = true;
    file_element->m_context->transformFixed = true;
    file_element->m_context->dontComputeSizeOfChildren = true;

    TextElement* text_file_type = new TextElement(f.is_directory ? m_stringDirectory : m_stringFile, 
        f.is_directory ? conf::fonts::emojis : conf::fonts::emoji_type, 30);
    text_file_type->m_vars.size.absolute.setAll({50.0f, 5.0f});
    text_file_type->m_vars.color.setAll(f.is_directory ? Color(255,233,153) : Color(100,100,100));
    text_file_type->m_vars.borderColor.setAll(Color(238,203,74));
    text_file_type->m_vars.borderWidth.setAll(f.is_directory ? 2.0f : 0.0f);
    text_file_type->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_file_type->m_context->sizeMode[1] = UISizeMode::FIXED;
    text_file_type->m_textPosAbs = {0.0f, f.is_directory ? 1.0f : 6.0f};
    text_file_type->m_textOriginRel = {0.5f, 1.0f};
    text_file_type->m_textPosRel = {0.5f, 1.0f};
    file_element->addChild(text_file_type);
    text_file_type->m_context->z_index = 10;

    UIElement* text_container = new UIElement();
    text_container->m_vars.flex.setAll(fileFieldsFlexSize[0].first);
    text_container->m_vars.size.absolute.setAll({fileFieldsFlexSize[0].second, 0.0f});
    text_container->m_vars.size.relative.setAll({0.0f, 1.0f});
    text_container->m_vars.color.setAll(Color(0,0,0,0));
    file_element->addChild(text_container);
    text_container->m_context->z_index = 10;

    TextElement* text_name = new TextElement(f.name.wstring(), conf::fonts::mono_r, 22);
    text_name->m_vars.size.relative.setAll({1.0f, 0.0f});
    text_name->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_name->m_vars.color.setAll(Color(40,40,40));
    text_name->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_name->m_context->sizeMode[1] = UISizeMode::FIXED;
    text_name->m_textOverflow = TextOverflow::ELLIPSIS;
    text_name->m_textPosAbs = {4.0f, 12.0f};
    text_container->addChild(text_name);

    TextElement* text_path = new TextElement(f.path.wstring(), conf::fonts::mono_r, 16);
    text_path->m_vars.size.relative.setAll({1.0f, 0.0f});
    text_path->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_path->m_vars.origin.relative.setAll({0.0f, 1.0f});
    text_path->m_vars.pos.relative.setAll({0.0f, 1.0f});
    text_path->m_vars.color.setAll(Color(120,120,120));
    text_path->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_path->m_context->sizeMode[1] = UISizeMode::FIXED;
    text_path->m_textOverflow = TextOverflow::ELLIPSIS;
    text_path->m_textPosAbs = {4.0f, -12.0f};
    text_path->m_textPosRel = {0.0f, 1.0f};
    text_path->m_textOriginRel = {0.0f, 1.0f};
    text_container->addChild(text_path);

    TextElement* text_size = new TextElement(f.is_directory ? "" : util::formatBytes(f.bytes), conf::fonts::mono_r, 22);
    text_size->m_vars.flex.setAll(fileFieldsFlexSize[1].first);
    text_size->m_vars.size.absolute.setAll({fileFieldsFlexSize[1].second, 0.0f});
    text_size->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_size->m_vars.color.setAll(Color(128,128,128));
    text_size->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_size->m_textPosAbs = {-10.0f, 0.0f};
    text_size->m_textPosRel = {1.0f, 0.0f};
    text_size->m_textOriginRel = {1.0f, 0.0f};
    file_element->addChild(text_size);
    text_size->m_context->z_index = 10;

    TextElement* text_date = new TextElement(util::formatDate(f.last_write_time), conf::fonts::mono_r, 22);
    text_date->m_vars.flex.setAll(fileFieldsFlexSize[2].first);
    text_date->m_vars.size.absolute.setAll({fileFieldsFlexSize[2].second, 0.0f});
    text_date->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_date->m_vars.color.setAll(Color(128,128,128));
    text_date->m_context->sizeMode[0] = UISizeMode::FIXED;
    text_date->m_textPosAbs = {-10.0f, 0.0f};
    text_date->m_textPosRel = {1.0f, 0.0f};
    text_date->m_textOriginRel = {1.0f, 0.0f};
    file_element->addChild(text_date);
    text_date->m_context->z_index = 10;

    return file_element;
}

void FileExplorerView::renameDetailedFileButton(UIElement* e, const File& f) {
    if (std::filesystem::is_directory(f.path)) {
        e->m_context->onClick[0] = [this, f](MouseContext& m) {
            m_model.setPath(f.path);
        };
    }
    else {
        const std::filesystem::path parent = f.path.parent_path();
        e->m_context->onClick[0] = [this, parent](MouseContext& m) {
            m_model.setPath(parent);
        };
    }
    if (conf::settings::smoothSort) {
        e->m_vars.pos.absolute.setDuration(0.2f);
    }
    else {
        e->m_vars.pos.absolute.setDuration(0.0f);
    }
    e->m_context->data_id = f.id;

    TextElement* text_type = (TextElement*)(e->m_children[0]);
    TextElement* text_name = (TextElement*)(e->m_children[1]->m_children[0]);
    TextElement* text_path = (TextElement*)(e->m_children[1]->m_children[1]);
    TextElement* text_size = (TextElement*)(e->m_children[2]);
    TextElement* text_date = (TextElement*)(e->m_children[3]);

    text_type->m_textPosAbs = {0.0f, f.is_directory ? 1.0f : 6.0f};
    text_type->m_text.setFont(f.is_directory ? conf::fonts::emojis : conf::fonts::emoji_type);
    text_type->m_vars.color.setAll(f.is_directory ? Color(255,233,153) : Color(100,100,100));
    text_type->m_vars.borderWidth.setAll(f.is_directory ? 2.0f : 0.0f);
    text_type->setString(f.is_directory ? m_stringDirectory : m_stringFile, false);
    text_name->setString(f.name.wstring());
    text_path->setString(f.path.wstring());
    text_size->setString(f.is_directory ? "" : util::formatBytes(f.bytes), false);
    text_date->setString(util::formatDate(f.last_write_time), false);
}

UIElement* FileExplorerView::createPathButton(const std::filesystem::path& path, const std::filesystem::path& name) {
    UIElement* path_element = new UIElement();
    path_element->m_vars.size.relative.setAll({0.0f, 1.0f});
    path_element->m_context->sizeMode[0] = UISizeMode::FIT_CONTENT;
    path_element->m_vars.padding.setAll(6.0f); 
    path_element->m_vars.color.setAll(Color(0,0,0,0));
    path_element->m_vars.color.setState(UIElementState::HOVER, Color(207,227,247));
    path_element->m_vars.color.setState(UIElementState::ACTIVE, Color(178,213,248));
    path_element->m_context->onClick.push_back([this, path](MouseContext& m) {
        m_model.setPath(path);
    });
    path_element->m_context->copyTransformToChildren = true;
    path_element->m_context->dontUpdateChildren = true;
    path_element->m_context->statelessChildren = true;

    TextElement* text_element = new TextElement(name.wstring(), conf::fonts::mono_r_semibold, 22);
    text_element->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_element->m_vars.color.setAll(Color(40,40,40));
    text_element->m_vars.origin.relative.setAll({0.0f, 0.5f});
    text_element->m_vars.pos.relative.setAll({0.0f, 0.5f});
    path_element->addChild(text_element);
    text_element->m_context->z_index = 10;

    return path_element;
}

void FileExplorerView::renamePathButton(UIElement* e, const std::filesystem::path& path, const std::filesystem::path& name) {
    e->m_context->onClick[0] = [this, path](MouseContext& m) {
        m_model.setPath(path);
    };
    TextElement* text = (TextElement*)e->m_children[0];
    text->setString(name.wstring(), false);
}

UIElement* FileExplorerView::createPathDelimeter() {
    TextElement* text_element = new TextElement("/", conf::fonts::mono_r_semibold, 22);
    text_element->setFixedHeight("WERTYUUIIOOPASDFGHJKLZXCVBNM");
    text_element->m_vars.color.setAll(Color(180,180,180));

    return text_element;
}