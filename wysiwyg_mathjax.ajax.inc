<?php
/**
 * Called through ajax(AHAH), uploading a formula and replacing
 * the upload form with a formula-details form, if successfully uploaded.
 */
function _wysiwyg_mathjax_entity_upload($node_form_build_id) {
  // Load the cache which includes the currently already uploaded files.
  rs_debug("node_form_build_id=$node_form_build_id");
  $cached_files = cache_get('wysiwyg_mathjax_' . $node_form_build_id);
  rs_debug("cached files=", $cached_files);

  // Yet no file, initialize the array.
  if (empty($cached_files)) {
    $cached_files = new stdClass();
    $cached_files->data = array();
  }
  $cached_files = $cached_files->data;

  // Handle new uploads. Create the files in the files table and make them temporary.
  // They will need to be made persistent on form submit in the hook_comment or hook_nodeapi
  $formula_details = _wysiwyg_mathjax_upload_file();
  rs_debug("formula_details=", $formula_details);
  if ($formula_details->filepath != '') {
    $options = array();
    if (variable_get('wysiwyg_mathjax_filename_as_title_default', FALSE)) {
      $options['title'] = check_plain($formula_details->filename);
    }
    // save the fid as used inline and generate a iid (inline id)
    $iid = _wysiwyg_mathjax_create_inline_entity($formula_details, TRUE, $options);
    rs_debug("got iid=", $iid);
    // we store the iid in the file object, so we can use it on submit of the node form
    // to update the inline-entry to be static and not temporary
    $formula_details->iid = $iid;
    $cached_files[$iid] = $formula_details;
    cache_set( 'wysiwyg_mathjax_' . $node_form_build_id, $cached_files, 'cache', CACHE_PERMANENT );
    return $iid;
  }
  //else
  return FALSE;
}

/**
 * Ajax callback to render a formula for the wysiwyg editor
 */
function _wysiwyg_mathjax_render_wysiwyg($iid) {
  $formula_obj = _wysiwyg_mathjax_load_inline_entity($iid);
  $output = theme('wysiwyg_mathjax_render_wysiwyg', $formula_obj, array());

  $GLOBALS['devel_shutdown'] = FALSE;
  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  print _wysiwyg_mathjax_to_js(array('status' => TRUE, 'data' => $output));
  exit();
}

/**
 * Ajax callback to render formulae for the wysiwyg editor
 */
function _wysiwyg_mathjax_render_wysiwyg_formulae($iids, $revisioned, $form_id) {
  $iids = split(',', $iids);
  $output = array();
  foreach ($iids as $iid) {
    // check access rights
    if(_wysiwyg_mathjax_access($iid, 'view') == FALSE) {
      $output[$idd] = 'You dont have the permission to view this formula';
      continue;
    }
    // else render the formula
    $formula_obj = _wysiwyg_mathjax_load_inline_entity($iid);
    $output[$iid] = theme('wysiwyg_mathjax_render_wysiwyg', $formula_obj, array('wysiwyg_placeholder' => '1'));
  }

  $GLOBALS['devel_shutdown'] = FALSE;
  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  print _wysiwyg_mathjax_to_js(array('status' => TRUE, 'data' => $output));
  exit();
}

/**
 * Helper to convert a inline-formula into a backend tag
 */
function _wysiwyg_mathjax_render_backend($iid, $arguments = "") {
  $formula_obj = _wysiwyg_mathjax_load_inline_entity($iid);
  return "[[wysiwyg_mathjax:$iid:$arguments]]";
}
