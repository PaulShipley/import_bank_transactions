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
* @author Paul Shipley <paul@paulshipley.com.au>
* @copyright 2019 Paul Shipley
* @license GPL
*/

define ('SS_IMPORTBANKTRANSACTIONS', 101<<8);

include_once($path_to_root . "/modules/import_bank_transactions/import_bank_transactions_class.php");
class hooks_import_bank_transactions extends hooks {
	var $module_name = 'import_bank_transactions'; 

	/*
		Install additional menu options provided by module
	*/
    function install_tabs($app) {
        $app->add_application(new import_bank_transactions_app);
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
