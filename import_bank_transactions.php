<?php
/**********************************************************************
    Copyright (C) Paul Shipley
	Released under the terms of the GNU General Public License, GPL, 
	as published by the Free Software Foundation, either version 3 
	of the License, or (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the License here <http://www.gnu.org/licenses/gpl-3.0.html>.
***********************************************************************/
/**********************************************************************
// Creator: Paul Shipley
// date_:   2019-01-09
// Title:   Import Bank Transactions
// Free software under GNU GPL
***********************************************************************/


$page_security = 'SA_IMPORTBANKTRANSACTIONS';
$path_to_root = "../..";
include_once ($path_to_root . "/includes/session.inc");
include_once ($path_to_root . "/includes/sysnames.inc"); //referencetype_list_row for determining next reference for source documents
include_once ($path_to_root . "/includes/main.inc"); //function page
include_once ($path_to_root . "/includes/ui.inc");
//include_once ($path_to_root . "/includes/ui/items_cart.inc"); //class 'items_cart' gl_items for classic tabular representation of journals
//include_once ($path_to_root . "/includes/ui/ui_input.inc");
//include_once ($path_to_root . "/includes/db/audit_trail_db.inc"); //add_audit_trail mandatory for all import transactions
//include_once ($path_to_root . "/gl/includes/db/gl_db_trans.inc"); // write journal entries; add_gl_tax_details; add_gl_trans
//include_once ($path_to_root . "/gl/includes/db/gl_db_bank_trans.inc"); //add_bank_trans
//include_once ($path_to_root . "/gl/includes/db/gl_db_bank_accounts.inc"); //get_bank_gl_account
//include_once ($path_to_root . "/gl/includes/db/gl_db_accounts.inc"); // gl_account_in_bank_accounts
//include_once ($path_to_root . "/gl/includes/gl_db.inc"); //link to other includes
//include_once ($path_to_root . "/includes/date_functions.inc"); //sql2date, is_date_in_fiscal_year
//include_once ($path_to_root . "/includes/data_checks.inc");
//include_once ($path_to_root . "/admin/db/company_db.inc"); //default control accounts
//include_once ($path_to_root . "/includes/ui/ui_controls.inc");
//include_once ($path_to_root . "/modules/import_transactions/includes/import_transactions.inc"); //functions used
//include_once ($path_to_root . "/modules/import_transactions/includes/import_sales_order_entry.inc"); // adaptation of sales_order_entry.php
//include_once ($path_to_root . "/modules/import_transactions/includes/import_sales_cart_class.inc"); // adaptation of cart class
//include_once ($path_to_root . "/modules/import_transactions/includes/import_sales_order_ui.inc"); // adaptation of sales_order_ui.inc
//include_once($path_to_root . "/gl/includes/ui/gl_journal_ui.inc"); display_import_items adapted from display_ gl_items

add_access_extensions();

//Turn these next two lines on for debugging
error_reporting(E_ALL);
ini_set("display_errors", "on");

//Set '$yes' to true if you are testing this module and you do not want to manually(phpmyadmin) delete previous test run records before each test run
//Ensure that your company has no important information in it as these will be deleted by means of all_delete function under import_transactions.inc
//Warning: Most records will be deleted if '$yes' set to true. Default must stay on false for normal operation.
//Recommended: Remove this next line after you are happy with testing.
//all_delete($yes = false);
$js = "";
if ($SysPrefs->use_popup_windows) 
    $js .= get_js_open_window(800, 500);
$help_context = "Import General Journals  / Deposits / Payments / Bank Statements / Sales Orders / Sales Invoices  <a href='spreadsheet_headers.html' target='_blank'>Help: Formats</a>";
page(_($help_context), false, false, "", $js);

global $Refs;
global $Ajax;

$filename = (isset($_GET['filename']) ? $_GET['filename'] : '');
if ($filename != "") {
    initialize_controls();
    $_POST['type']             = ST_JOURNAL;
    $_FILES['imp']['name']     = $filename;
    $_FILES['imp']['tmp_name'] = $filename;
    $_POST['sep']              = ",";
    $_POST['trial']            = false;
}

$curEntryId = false;

if ((isset($_POST['type']))) {
    $type = $_POST['type'];
} // if (isset($_POST['type']))

function initialize_controls()
//initialize dropdown boxes
{
    //$myrow = get_company_prefs();
    if (!isset($_POST['type'])) $_POST['type'] = null;
    if (!isset($_POST['sep'])) $_POST['sep'] = ",";
    if (!isset($_POST['bank_account'])) $_POST['bank_account'] = _("Current account");
    if (!isset($_FILES['imp'])) $_FILES['imp'] = null;
    if (!isset($_POST['trial'])) $_POST['trial'] = !null;
}

function show_table_section_import_settings() {
    table_section_title(_("Import Settings"), 2);
    bank_accounts_list_cells(_("Bank Account:"), 'bank_account', $selected_id = "", $_POST['bank_account']);
    show_table_section_bank_account_number();
}

function show_table_section_bank_account_number() {
    $bank = $_POST['bank_account'];
    $account = get_bank_gl_account($bank);
    table_section_title(_("Bank Accounts"), 2);
    label_row(_("Account number: "), $account, 'colspan=1');
}

function show_table_section_csv_separator() {
    table_section_title(_("CSV fields are separated by: "), 2);
    text_cells(_("Field Separator:"), 'sep', $_POST['sep'], 2, 1, $title = "", 'colspan=1');
}

function show_table_section_trial_or_final() {
    table_section_title(_("Trial-run (tick) or Final-run (no tick)"), 2);
    check_row(_("Trial-run before importing. Do NOT process into database."), 'trial', $_POST['trial'] = !null, $submit_on_change = false, $title = "");
    label_row(_("Import File:"), "<input type='file' id='imp' name='imp'>", 'colspan=1');
}

// User Interface
start_form(true);
div_start('_main_table');
initialize_controls();
start_table(TABLESTYLE2, "width = 95%"); //inner table
show_table_section_import_settings();
show_table_section_csv_separator();
show_table_section_trial_or_final();
end_table(1);
div_end('_main_table');
submit_center('import', "Process", true, false, 'process', ICON_OK);
end_form();
end_page();




