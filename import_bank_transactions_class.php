<?php
/**
* Import Bank Transactions module
*
* @author Paul Shipley <paul@paulshipley.com.au>
* @copyright 2019 Paul Shipley
* @license GPL
*/

class import_bank_transactions_app extends application
{

	function __construct()
	{
		$this->add_lapp_function(0, _("Import Bank Transactions"), 
			'/modules/import_bank_transactions/import_bank_transactions.php?', 'SA_IMPORTBANKTRANSACTIONS', MENU_TRANSACTION);

        $this->add_extensions();

	}

}
