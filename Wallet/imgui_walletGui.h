#include "stdafx.h"
#pragma once

namespace ImGui
{
	// ���� ������
	void BillWindow(AccountsList& accountList, bool* p_open)
	{
		const char* operations[] = { "Increase","Decrease" };

		std::vector<std::string> nameOfAccounts;
		std::vector<std::string> billIds;

		for (int32_t iii = 0; iii < accountList.getNumberOfAccounts(); ++iii)
		{
			nameOfAccounts.push_back(accountList.getAccountList().at(iii)->getName() +
				"##" +
				static_cast<char>(accountList.getAccountList().at(iii)->getAccountId()));
		}

		static int32_t index = 0;
		static int32_t value = 0;
		static int32_t id = 0;
		static int32_t d(0), mon(0), y(0), h(0), mins(0), s(0);

		static Bills::eOperationType operationType = Bills::eOperationType::DECREASE;

		{
			ImGui::Begin("Bill Menu", p_open);

			static int accountIdx = 0;
			static int prevAccIdx = 0;																// TODO: �������� ���������
			static int billIdx = 0;
			const char* previewValue = "NULL";
			const char* previewBillId = "NULL";

			if (!nameOfAccounts.empty() && !accountList.getAccountList().at(accountIdx)->getBillList().getList().empty())
			{
				for (int32_t iii = 0; iii < accountList.getAccountList().at(accountIdx)->getBillList().getNumberOfBills(); ++iii)
				{
					billIds.push_back(std::to_string(accountList.getAccountList().at(accountIdx)->getBillList().getList().at(iii)->getCurrentId()));
				}
				billIds.shrink_to_fit();
			}

			if (!billIds.empty())
				previewBillId = billIds.at(billIdx).c_str();

			if (!nameOfAccounts.empty())
				previewValue = nameOfAccounts.at(accountIdx).c_str();

			ImGui::Text("Size of AccountList: %d", nameOfAccounts.size());								// DEBUG!

			if (ImGui::BeginCombo("Choice Account for Operation", previewValue))
			{
				for (int iii = 0; iii < nameOfAccounts.size(); ++iii)
				{
					const bool isSelected = (accountIdx == iii);

					if (ImGui::Selectable(nameOfAccounts.at(iii).c_str(), isSelected))
						accountIdx = iii;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// ImGui::Text("Size: %d, Capacity: %d", billIds.size(), billIds.capacity());


			ImGui::InputInt("Value", &value);

			ImGui::Combo("Operation Type", &index, operations, IM_ARRAYSIZE(operations));

			ImGui::InputInt("Day", &d);
			ImGui::InputInt("Month", &mon);
			ImGui::InputInt("Year", &y);

			ImGui::InputInt("Hours", &h);
			ImGui::InputInt("Minutes", &mins);
			ImGui::InputInt("Seconds", &s);

			if (ImGui::Button("Add Bill") && !accountList.getAccountList().empty())
			{
				Time tempTime;
				tempTime.setDate(d, mon, y);
				tempTime.setTime(h, mins, s);
				accountList.getAccountList().at(accountIdx)->getBillList().addBill(Bills::Bill(value, Bills::findEOperation(index), tempTime));
				if (index == 0)
					accountList.getAccountList().at(accountIdx)->addValue(value);
				else
					accountList.getAccountList().at(accountIdx)->reduceValue(value);
				IO::saveAccounts(accountList);
			}

			if (ImGui::BeginCombo("##Bill Id to delete", previewBillId))
			{
				for (int iii = 0; iii < billIds.size(); ++iii)
				{
					const bool isSelected = (billIdx == iii);
					const char* temp = billIds.at(iii).c_str();

					if (ImGui::Selectable(temp, isSelected))
						billIdx = iii;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove Bill"))
			{
				[](AccountsList& acc, int32_t accId, Bills::eOperationType type, int32_t value)
				{
					switch (type)
					{
					case Bills::eOperationType::INCREASE:
						acc.getAccountList().at(accId)->reduceValue(value);
						break;
					case Bills::eOperationType::DECREASE:
						acc.getAccountList().at(accId)->addValue(value);
						break;
					}
				}
					(
						accountList, accountIdx,
						accountList.getAccountList().at(accountIdx)->getBillList().getList().at(billIdx)->getOperationType(),
						accountList.getAccountList().at(accountIdx)->getBillList().getList().at(billIdx)->getValue()
						);
				accountList.getAccountList().at(accountIdx)->getBillList().removeBill(billIdx);
				billIdx = 0;
				IO::saveAccounts(accountList);
			}

			ImGui::End();
		}
	}

	// ���� �������� ��������
	void CreateAccount(AccountsList& accList, bool* winOpen)												// TODO: ������ ����� �������� ������
	{
		static int32_t value = 0;
		static int32_t i = 0;
		static std::string name = "Account Name 1";
		const char* typeOfWallet[] = { "Dollar","Euro","Rubles" };

		{
			ImGui::SetNextWindowSize(ImVec2(350, 150));
			ImGui::Begin("Create new account", winOpen, ImGuiWindowFlags_NoResize);

			ImGui::InputText("Name of Account", &name);
			ImGui::Combo("Type of Wallet", &i, typeOfWallet, IM_ARRAYSIZE(typeOfWallet));
			ImGui::InputInt("Value", &value);

			if (ImGui::Button("Create Account"))
			{
				accList.addAccount(Account(name, findEWallet(i), value));										// TODO: ���������� � ������; �����������, ����� �������� ��������� ����� (Account)
				IO::saveAccounts(accList);
			}

			ImGui::End();
		}
	}

	// ���� � ����������
	void Accounts(AccountsList& accList, bool* showAccounts)
	{
		// �������� ����
		ImGui::SetNextWindowSize(ImVec2(700, 400));
		ImGui::Begin("Accounts", showAccounts, ImGuiWindowFlags_NoResize);

		// ���� ���������� ������ � ������� ��������
		for (int index = 0; index < accList.getNumberOfAccounts(); ++index)
		{
			std::string name = accList.getAccountList().at(index)->getName();
			eWallet walletType = accList.getAccountList().at(index)->getWallet();
			int32_t value = accList.getAccountList().at(index)->getValue();
			int32_t id = accList.getAccountList().at(index)->getAccountId();
			Bills::BillList& billList = accList.getAccountList().at(index)->getBillList();

			// ���� � ������� �� �������� � ��� �������
			{
				std::string childName = name + "##" + static_cast<char>(id);
				if (ImGui::CollapsingHeader(childName.c_str()))
				{
					ImGui::BeginChild(childName.c_str(), ImVec2(0, 250), true);

					ImGui::Text("Wallet Type: "); ImGui::SameLine();
					ImGui::Text(eWalletToString(walletType).c_str());
					ImGui::Text("Value: %d", value);
					ImGui::Text("Number of bills: %d", billList.getNumberOfBills());

					if (ImGui::BeginTable("Bills", 5, ImGuiTableFlags_Borders))
					{
						ImGui::TableSetupColumn("Id");
						ImGui::TableSetupColumn("Type of Operation");
						ImGui::TableSetupColumn("Value");
						ImGui::TableSetupColumn("Time");
						ImGui::TableSetupColumn("Date");
						ImGui::TableHeadersRow();

						auto operationTypeToStr
						{ [](Bills::eOperationType opType)
							{
								switch (opType)
								{
								case Bills::eOperationType::DECREASE:
									return "Decrease";
								case Bills::eOperationType::INCREASE:
									return "Increase";
								break;
								}
							}
						};

						for (int row = 0; row < billList.getNumberOfBills(); ++row)
						{
							ImGui::TableNextRow();
							for (int column = 0; column < 5; ++column)
							{
								ImGui::TableSetColumnIndex(column);
								switch (column)
								{
								case 0:
									ImGui::Text("%d", billList.getList().at(row)->getCurrentId());
									break;
								case 1:
									ImGui::Text(operationTypeToStr(billList.getList().at(row)->getOperationType()));
									break;
								case 2:
									ImGui::Text("%d", billList.getList().at(row)->getValue());
									break;
								case 3:
									ImGui::Text(billList.getList().at(row)->getOperationTime().stringTime().c_str());
									break;
								case 4:
									ImGui::Text(billList.getList().at(row)->getOperationTime().stringDate().c_str());
									break;
								}
							}
						}
						ImGui::EndTable();
					}

					if (ImGui::Button("Delete this account"))
					{
						accList.removeAccount(id);
						IO::saveAccounts(accList);
					}

					ImGui::EndChild();
				}
			}
		}

		ImGui::End();
	}
}