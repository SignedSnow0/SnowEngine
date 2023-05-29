#include "LogView.h"

#include <imgui.h>

#include "ImGuiControls.h"

namespace SnowEditor
{
	LogView::LogView()
	{
		mSearch.resize(1024);
	}

	void LogView::Draw()
	{
		const ImVec2 padding{ ImGui::GetStyle().WindowPadding };
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		if (ImGui::Begin("Log"))
		{
			ImGui::SetCursorPos({ ImGui::GetCursorPos().x + padding.x, ImGui::GetCursorPos().y + padding.y });
			ImGui::InputTextWithHint("##Search", "Search...", mSearch.data(), mSearch.size());
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Green));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::GreenHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::GreenPressed));
			ImGui::ToggleButton("##Trace", &mShowTrace, ImVec2(0.0f, 0.0f), ImDrawFlags_RoundCornersAll);
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Blue));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::BlueHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::BluePressed));
			ImGui::ToggleButton("##Warning", &mShowWarning, ImVec2(0.0f, 0.0f), ImDrawFlags_RoundCornersAll);
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Red));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::RedHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::RedPressed));
			ImGui::ToggleButton("##Error", &mShowError, ImVec2(0.0f, 0.0f), ImDrawFlags_RoundCornersAll);
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Green));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::GreenHovered));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::GreenPressed));
			ImGui::ToggleButton("##Debug", &mShowDebug, ImVec2(0.0f, 0.0f), ImDrawFlags_RoundCornersAll);
			ImGui::PopStyleColor(3);

			if (ImGui::BeginTable("##LogMessages", 5, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("Severity");
				ImGui::TableSetupColumn("Message");
				ImGui::TableSetupColumn("File");
				ImGui::TableSetupColumn("Line");
				ImGui::TableSetupColumn("Count");
				ImGui::TableHeadersRow();

				for (const auto& [message, severity, file, line, repeatCount] : SnowEngine::Logger::GetMessages())
				{
					b8 show = true;
					switch (severity)
					{
						case SnowEngine::LogSeverity::Trace:
							if (!mShowTrace)
								show = false;
							break;
						case SnowEngine::LogSeverity::Warning:
							if (!mShowWarning)
								show = false;
							break;
						case SnowEngine::LogSeverity::Error:
							if (!mShowError)
								show = false;
							break;
						case SnowEngine::LogSeverity::Debug:
							if (!mShowDebug)
								show = false;
							break;
					}
					if (!show)
						continue;

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::TextColored(GetSeverityColor(severity), GetSeverityString(severity).c_str());
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(message.c_str());
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(file.c_str());
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginTooltip())
					{
						ImGui::TextUnformatted(file.c_str());
						ImGui::EndTooltip();
					}
					ImGui::TableNextColumn();
					ImGui::Text("%d", line);
					ImGui::TableNextColumn();
					ImGui::Text("%d", repeatCount);
				}	

				ImGui::EndTable();
			}

		}
		ImGui::End();

		ImGui::PopStyleVar();
	}

	std::string LogView::GetSeverityString(const SnowEngine::LogSeverity severity)
	{
		switch (severity)
		{
		case SnowEngine::LogSeverity::Trace: return "Trace";
		case SnowEngine::LogSeverity::Warning: return "Warning";
		case SnowEngine::LogSeverity::Error: return "Error";
		case SnowEngine::LogSeverity::Debug: return "Debug";
		}

		return "Unknown";
	}

	ImVec4 LogView::GetSeverityColor(const SnowEngine::LogSeverity severity)
	{
		switch (severity)
		{
		case SnowEngine::LogSeverity::Trace: return SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Green);
		case SnowEngine::LogSeverity::Warning: return SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Blue);
		case SnowEngine::LogSeverity::Error: return SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Red);
		case SnowEngine::LogSeverity::Debug: return SnowEngine::DarkColors::HexToSrgb(SnowEngine::DarkColors::Green);
		}

		return ImGui::GetStyle().Colors[ImGuiCol_Text];
	}
}
