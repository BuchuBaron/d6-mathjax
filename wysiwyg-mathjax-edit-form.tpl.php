<?php
/**
 * @file wysiwyg-mathjax-edit-form.tpl.php
 *
 * This template handles the layout of the formula edit form in wysiwyg_mathjax
 *
 * Variables available:
 * - $form : holds the form api form array
 */

global $base_url;
if (!empty($q)):
  // This ensures that, if clean URLs are off, the 'q' is added first so that
  // it shows up first in the URL.
  print $q;
endif;
// It doesn't look like theme files work with jquery dialogs.  This markup has gone
// straight into the form for now
$contents = '<table border="0"><tr><td width="50%">';
$contents .= drupal_render($form['formula_edit_details']['wysiwyg_mathml_text']);
$contents .= "</td>\n<td>";
$contents  .= drupal_render($form);
$contents .= "</td>\n</tr></table>";

print $contents;
?>
