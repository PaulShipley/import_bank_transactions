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
/**
* Import Bank Transactions module
*
* This is a variation of the gl/gl_bank.php functionality modified to import bank transactions
* Functionality is identical to the orginal, except that the date, bank account, amount and memo fields
* will be pre-populated. The payment/deposit can then be processed as usual. Once committed, the next
* transaction will be shown. Both payments and deposits will be processed in date order, earlist first.
*
* Note that the pre-populated data can be ammended, and the tranaction can even not be processed,
* but this is unlikly to be what you want.
*
* @author Paul Shipley <paul@paulshipley.com.au>
* @copyright 2019 Paul Shipley
* @license GPL
*/

$page_security = 'SA_IMPORTBANKTRANSACTIONS';
$path_to_root  = "../..";
include_once ($path_to_root . "/includes/session.inc");
include_once ($path_to_root . "/includes/sysnames.inc");
include_once ($path_to_root . "/includes/main.inc");
include_once ($path_to_root . "/includes/ui.inc");
include_once ($path_to_root . "/includes/date_functions.inc");
include_once ($path_to_root . "/modules/import_bank_transactions/import_bank_transactions.inc");

add_access_extensions();

//Turn these next two lines on for debugging
error_reporting(E_ALL);
ini_set("display_errors", "on");


// Process a new import file
$trans_finished= FALSE;
$ImportBankTrans = new import_bank_transaction();

if (isset($_GET['ImportTrans'])) {
    $import_id = $_GET['import_id'];

    $ImportBankTrans->delete_transaction($import_id);
}

if (isset($_GET['ImportTrans']) || $ImportBankTrans->is_transactions()) {
    list($import_id, $acct, $date, $amt, $memo) = $ImportBankTrans->get_next_transaction();

    // process next transaction, fall through if none left
    if (isset($import_id)) {
        $date = sql2date($date);

        if ($amt > 0) {
            meta_forward("$path_to_root/modules/import_bank_transactions/import_bank_gl_bank.php", "NewDeposit=yes&import_id=".$import_id."&bank_account=".$acct."&date=".$date."&amount=".$amt."&memo=".urlencode($memo));
        } else {
            $amt = - $amt;
            meta_forward("$path_to_root/modules/import_bank_transactions/import_bank_gl_bank.php", "NewPayment=yes&import_id=".$import_id."&bank_account=".$acct."&date=".$date."&amount=".$amt."&memo=".urlencode($memo));
        }
    } else {
        $trans_finished = TRUE;
    }
}


// Setup page
$js = "";
if ($SysPrefs->use_popup_windows)
$js .= get_js_open_window(800, 500);
$help_context = "Import Bank Statements";
page(_($help_context), false, false, "", $js);

global $Refs;
global $Ajax;

$filename = (isset($_GET['filename']) ? $_GET['filename'] : '');
if ($filename != "") {
    initialize_controls();
    $_POST['type'] = ST_JOURNAL;
    $_FILES['imp']['name'] = $filename;
    $_FILES['imp']['tmp_name'] = $filename;
    $_POST['sep'] = ",";
    $_POST['trial'] = false;
}


// All transactions processed
if ($trans_finished) {
    display_notification_centered( _("All bank transactions processed"));

    hyperlink_params($_SERVER['PHP_SELF'], _("Import another file"), "");

    display_footer_exit();
}


// Import file processed, show next steps
if (isset($_FILES['imp']) && $_FILES['imp']['name'] != '') {
    $filename    = $_FILES['imp']['tmp_name'];
    $sep         = $_POST['sep'];
    $date_format = $_POST['date_format'];
    $date_sep    = $_POST['date_sep'];
    $trial       = (isset($_POST['trial']) ? $_POST['trial'] : false);
    $bank_account = isset($_POST['bank_account']) ? $_POST['bank_account'] : "";

    $ImportBankTrans = new import_bank_transaction();
    $num_trans       = $ImportBankTrans->load_transactions($bank_account, $filename, $sep, $date_format, $date_sep, $trial);

    display_notification_centered(sprintf(_("%d bank transactions imported"), $num_trans));

    if ($num_trans != 0) {
        hyperlink_params($_SERVER['PHP_SELF'], _("Process Transactions"), "ImportTrans=yes&import_id=0");
    }

    hyperlink_params($_SERVER['PHP_SELF'], _("Import another file"), "");

    display_footer_exit();
}


// Show main UI
function initialize_controls()
{
    //initialize dropdown boxes
    if (!isset($_POST['type'])) $_POST['type'] = null;
    if (!isset($_POST['sep'])) $_POST['sep'] = ",";
    if (!isset($_POST['bank_account'])) $_POST['bank_account'] = _("Current account");
    if (!isset($_FILES['imp'])) $_FILES['imp'] = null;
    if (!isset($_POST['trial'])) $_POST['trial'] = !null;
}

function show_table_section_import_settings()
{
//    table_section_title(_("Import Settings"), 2);
    table_section_title(_("Bank Account"), 2);
    bank_accounts_list_cells(_("Bank Account:"), 'bank_account', $selected_id = "", $_POST['bank_account']);
    show_table_section_bank_account_number();
}

function show_table_section_bank_account_number()
{
    $bank    = $_POST['bank_account'];
    $account = get_bank_gl_account($bank);
    label_row(_("Account number: "), $account, 'colspan=1');
}

function show_table_section_csv_separator()
{
    table_section_title(_("CSV fields - <a href='csv_file_format.html' target='_blank'>Help CSV File Format</a>"), 2);
    text_cells(_("Field Separator:"), 'sep', $_POST['sep'], 2, 1, $title = "", 'colspan=1');
}

function show_table_section_trial_or_final()
{
    table_section_title(_("Trial-run (tick) or Final-run (no tick)"), 2);
    check_row(_("Trial-run before importing. Do NOT process into database."), 'trial', $_POST['trial'] = !null, $submit_on_change = false, $title = "");
    label_row(_("Import File:"), "<input type='file' id='imp' name='imp'>", 'colspan=1');
}

start_form(true);
div_start('_main_table');
initialize_controls();
start_table(TABLESTYLE2, "width = 95%"); //inner table
show_table_section_import_settings();
show_table_section_csv_separator();
dateformats_list_row(_("Date Format:"), "date_format", user_date_format());
dateseps_list_row(_("Date Separator:"), "date_sep", user_date_sep());
show_table_section_trial_or_final();
end_table(1);
div_end('_main_table');
submit_center('import', "Process", true, false, '', ICON_OK);
end_form();
end_page();
