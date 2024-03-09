#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <iostream>
#include <windows.h>

#include "panel.h"
#include "state.h"
#include "machine.h"
#include "debug.h"
#include "gui.h"

// Controller Emulation
extern bool running;

using namespace std;

#define FONT_SIZE 20
#define ELEMENT_PER_ROW 8
#define SIZE 256

extern byte RAM[RAM_SIZE];

void openFileDialog(std::string &fileName)
{
    // common dialog box structure, setting all fields to 0 is important
    OPENFILENAME ofn = {0};
    TCHAR szFile[9600] = {0};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // <-- maybe needing HANDLE here ?
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("json files\0*.json\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        std::cout << "file selected : " << ofn.lpstrFile << std::endl;
        fileName = ofn.lpstrFile;
    }
}

void Debug_Hexdump(uint8_t *data)
{
    int ROWS = SIZE / 16;

    for (int row = 0; row < ROWS; row++)
    {
        ImGui::TableNextRow();

        int address = row * 16;
        char adr[64];

        // Address
        sprintf(adr, "%04X: ", address);
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(adr);
        adr[0] = '\0';

        // Data in Hex
        for (int i = 0; i < ELEMENT_PER_ROW; i++)
        {
            sprintf(adr + strlen(adr), "%02X%02X ", data[address + i * 2], data[address + i * 2 + 1]);
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(adr);

        // Data in ASCII
        adr[0] = '\0';
        for (int i = 0; i < ELEMENT_PER_ROW * 2; i++)
        {
            char c = data[address + i];
            if (c < 32 || c > 126)
            {
                c = '.';
            }
            sprintf(adr + strlen(adr), "%c", c);
        }
        ImGui::TableSetColumnIndex(2);
        ImGui::Text(adr);
    }
}

int Debug_init()
{
    GUI_init();
    return 0;
}

void Debug_loop()
{

    while (running)
    {
        GUI_NewFrame();

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Geral"))
            {
                ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

                if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                {
                    std::string fileName;
                    openFileDialog(fileName);
                    cout << fileName << endl;
                }
                ImGui::SameLine();
                ImGui::Text("counter = %d", 0);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Memoria RAM"))
            {

                if (ImGui::BeginTable("table1", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBodyUntilResize))
                {
                    // We could also set ImGuiTableFlags_SizingFixedFit on the table and all columns will default to ImGuiTableColumnFlags_WidthFixed.
                    ImGui::TableSetupColumn("Endereço", ImGuiTableColumnFlags_WidthFixed, 100.0f); // Default to 100.0f
                    ImGui::TableSetupColumn("Dado", ImGuiTableColumnFlags_WidthFixed, 400.0f);     // Default to 400.0f
                    ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 400.0f);    // Default to auto
                    ImGui::TableHeadersRow();

                    Debug_Hexdump(RAM);

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Cucumber"))
            {
                ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        GUI_Render();
    }
}

void Debug_destroy()
{
    GUI_Destroy();
}

const char *Debug_get_mnemonic(int opcode)
{
    // Switch based on the opcode
    switch (opcode)
    {
    case 0x80:
        return "LIMPO";
    case 0x81:
        return "UM";
    case 0x82:
        return "CMP1";
    case 0x83:
        return "CMP2";
    case 0x84:
        return "LIM";
    case 0x85:
        return "INC";
    case 0x86:
        return "UNEG";
    case 0x87:
        return "LIMP1";
    case 0x88:
        return "PNL_0";
    case 0x89:
        return "PNL_1";
    case 0x8A:
        return "PNL_2";
    case 0x8B:
        return "PNL_3";
    case 0x8C:
        return "PNL_4";
    case 0x8D:
        return "PNL_5";
    case 0x8E:
        return "PNL_6";
    case 0x8F:
        return "PNL_7";
    case 0x90:
        return "ST_0";
    case 0x91:
        return "STM_0";
    case 0x92:
        return "ST_1";
    case 0x93:
        return "STM_1";
    case 0x94:
        return "SV_0";
    case 0x95:
        return "SVM_0";
    case 0x96:
        return "SV_1";
    case 0x97:
        return "SVM_1";
    case 0x98:
        return "PUL";
    case 0x99:
        return "TRE";
    case 0x9A:
        return "INIB";
    case 0x9B:
        return "PERM";
    case 0x9C:
        return "ESP";
    case 0x9D:
        return "PARE";
    case 0x9E:
        return "TRI";
    case 0x9F:
        return "IND";
    case 0xD1:
        return "SH/RT/XOR/NAND";
    case 0xD2:
        return "XOR";
    case 0xD4:
        return "NAND";
    case 0xD8:
        return "SOMI";
    case 0xDA:
        return "CARI";
    default:
        // Switch based on the opcode's most significant nibble
        switch (opcode & 0xF0)
        {
        case 0x00:
            return "PLA";
        case 0x10:
            return "PLAX";
        case 0x20:
            return "ARM";
        case 0x30:
            return "ARMX";
        case 0x40:
            return "CAR";
        case 0x50:
            return "CARX";
        case 0x60:
            return "SOM";
        case 0x70:
            return "SOMX";
        case 0xA0:
            return "PLAN";
        case 0xB0:
            return "PLAZ";
        case 0xC0:
            return "IO";
        case 0xE0:
            return "SUS";
        case 0xF0:
            return "PUG";
        default:
            return "PARADO";
        }
    }
}
// Helper functions
