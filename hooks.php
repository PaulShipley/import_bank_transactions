<?php
/**********************************************************************
// Creator: Paul Shipley
// date_:   2019-01-09
// Title:   Import Bank Transactions
// Free software under GNU GPL
***********************************************************************/

define ('SS_IMPORTBANKTRANSACTIONS', 101<<8);

include_once($path_to_root . "/modules/import_bank_transactions/import_bank_transactions_class.php");
class hooks_import_bank_transactions extends hooks {
	var $module_name = 'import_bank_transactions'; 

	/*
		Install additional menu options provided by module
	*/
    function install_tabs($app) {
        $app->add_application(new import_bank_transaction_app);
	}

	function install_options($app) {
		global $path_to_root;

		switch($app->id) {
			case 'GL':
				$app->add_lapp_function(0, _("Import Bank Transactions"), 
					$path_to_root.'/modules/import_bank_transactions/import_bank_transactions.php', 'SA_IMPORTBANKTRANSACTIONS', MENU_TRANSACTION);
		}
	}

    function install_access()
    {
        $security_sections[SS_IMPORTBANKTRANSACTIONS] =    _("Import Bank Transactions");

        $security_areas['SA_IMPORTBANKTRANSACTIONS'] = array(SS_IMPORTTRANSACTIONS|101, _("Import Bank Transactions"));

        return array($security_areas, $security_sections);
    }

}
