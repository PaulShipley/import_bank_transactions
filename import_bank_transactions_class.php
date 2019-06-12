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

class import_bank_transactions_app extends application
{

	function __construct()
	{
		$this->add_lapp_function(0, _("Import Bank Transactions"), 
			'/modules/import_bank_transactions/import_bank_transactions.php?', 'SA_IMPORTBANKTRANSACTIONS', MENU_TRANSACTION);

        $this->add_extensions();

	}

}
