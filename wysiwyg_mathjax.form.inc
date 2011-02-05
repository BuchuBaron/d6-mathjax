<?php
/**
 * Implementation of hook_form_alter().
 */
function wysiwyg_mathjax_form_alter(&$form, $form_state, $form_id) {
  if ($form['#id'] == 'node-form' || $form['#id'] == 'comment-form' ) {
    if (variable_get('wysiwyg_mathjax_skip_dependencies_check', TRUE)) {
      _wysiwyg_mathjax_check_dependencies();
    }
    // SO UGLY: we have to use this, as FAPI has a special case with cached forms
    // If you save or preview the form and a validation error occurs, form_alter is not called
    // in that case all our javascript would be missing
    $form['#after_build'][] = '_wysiwyg_mathjax_prepare_main_form';
  }
}

/**
 * Adding jquery_ui_dialog javascript / css definitions to the form.
 */
function _wysiwyg_mathjax_prepare_main_form(&$form, $form_state) {
  drupal_add_js('misc/jquery.form.js');

  $settings = array(
    'current_form' => $form['form_build_id']['#value'],
    // only set revisions new, when we actually edit a node. It does not make sense for new node nor for comments
    'revisions' => (int) (($form['#id'] != 'comment-form') && _wysiwyg_mathjax_revisions_activated($form['#node']->type) && !empty($form['#node']->nid))
  );

  drupal_add_js(array('wysiwyg_mathjax' => $settings), 'setting');
  jquery_ui_dialog_parent_js();
  return $form;
}

/**
 * Returns the general mathML upload form.
 * This form is shown before a formula has been uploaded,
 * afterwards it is replaced by the formula details form
 */
function wysiwyg_mathjax_upload_form(&$form_state, $parent_build_id) {
  $m = drupal_get_path('module', 'wysiwyg_mathjax');
  $parent_build_id = check_plain($parent_build_id);
  jquery_ui_dialog_child_js();
  drupal_set_title(t('Upload MathML'));
  drupal_add_js("$m/js/wysiwyg_mathjax_uploadform.js");

  $form = array();
  $form['#attributes'] = array('enctype' => "multipart/form-data");
  $form['file_upload'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload MathML file'),
    '#prefix' => '<div id="file_upload_group">',
    '#suffix' => '</div>'
  );

  $form['parent_build_id'] = array(
    '#type' => 'value',
    '#value' => $parent_build_id
  );

  $form['file_upload']['wysiwyg_mathjax_file'] = array(
    '#type' => 'file',
    '#title' => t('Select a MathML file to upload'),
    '#size' => 22,
  );
  return $form;
}

function wysiwyg_mathjax_upload_form_validate(&$form, &$state) {
  $iid = _wysiwyg_mathjax_entity_upload($state['values']['parent_build_id']);
  if ($iid == FALSE) {
    form_set_error('wysiwyg_mathjax_file', t('This file could not be uploaded'));
    return;
  }
  
  // If the upload was fine, redirect to the details form
  $state['submitted'] = TRUE;
  $state['values']['iid'] = $iid;
  $state['values']['redirect'] = 'wysiwyg_mathjax/edit/' . $iid .'/' . $state['values']['parent_build_id'] . '?XDEBUG_SESSION_START=1';
}

function wysiwyg_mathjax_upload_form_submit($form, &$form_state) {
  $form_state['redirect'] = array($form_state['values']['redirect']);
}

/**
 * Called when a formula was uploaded and the details form should be shown.
 */
function wysiwyg_mathjax_edit_form(&$form_state, $iid, $node_form_build_id, $rebuild_entity = FALSE) {
  jquery_ui_dialog_child_js(array('width' => 740));
  drupal_set_title(t('Formula details'));

  $formula_obj = _wysiwyg_mathjax_load_inline_entity($iid);
  if ($formula_obj == FALSE) {
    drupal_set_message(t('Could not find formula in the database'), 'warning');
    return array();
  }

  $defaults = array(
    'filepath' => $formula_obj->filepath,
    'alignment' => $formula_obj->alignment,
    'style' => $formula_obj->style,
    'title' => $formula_obj->title,
  );
  $attributes = array(
    'class' => 'wysiwyg_mathjax_preview',
  );

  // Create the formula detail form.
  $form['#cache'] = TRUE;
  $form['#attributes'] = array(
    'enctype' => "multipart/form-data",
  );
  // Render the formula for the preview in the form. (yeah right!)
  $formula = theme('imagecache', 'wysiwyg_mathjax_preview', $defaults['filepath'], $iid, $defaults['title'], $attributes, TRUE, FALSE);
  // Load the details form.
  $form += _wysiwyg_mathjax_details_form($formula, $iid, $defaults);
  $form['rebuild_iid'] = array(
    '#type' => 'value',
    '#value' => FALSE,
  );

  if ($rebuild_entity != FALSE) {
    $form['rebuild_iid']['#value'] = TRUE;
  }
  return $form;
}

function wysiwyg_mathjax_edit_form_validate($form, &$state) {
  if ((array_key_exists('revisions', $_POST) && $_POST['revisions'] == 1) || $state['values']['rebuild_iid'] == TRUE) {
    // As this node is going to have a new revision AND we just EDITED a formula (NOT updated)
    // we create a new inline entity with the new settings and dont touch the old one.
    // This way the old inline formula stays the same in older revisions.

    // We are asked to actually create a new iid out of specific current one. This is needed e.g. for formula selected out of the browser.
    // The formula selected there is passed with its iid, to load all its defaults. But now we need to create a new iid for the
    // new use of the formula so we can have our own settings.
    $old_iid = $state['values']['iid'];
    $old_formula_obj = _wysiwyg_mathjax_load_inline_entity($old_iid);
    // Create a new inline entity and save it to our form_state
    // so the submit hanlder will handle it the right entity
    // Dont be confused here, this create handler ignores the current iid, so always creates
    // a new one based on the current meta-data
    $new_iid = _wysiwyg_mathjax_create_inline_entity($old_formula_obj, TRUE);
    $state['values']['iid'] = $new_iid;
  }
}

/**
 * Handles the submit of the edit form
 * Adds the details into the database for the inline-id
 */
function wysiwyg_mathjax_edit_form_submit($form, &$state) {
  $update = array(
    'iid',
  );
  $record = array(
    'style' => $state['values']['style'],
    'alignment' => $state['values']['alignment'],
    'title' => $state['values']['title'],
    'iid' => $state['values']['iid'],
  );

  // Update the details of the inline entity.
  drupal_write_record('wysiwyg_mathjax_entity', $record, $update);
  // Call all our api hooks.
  module_invoke_all('wysiwyg_mathjax_entity_save', $state['values'], $state['values']['iid']);
  $GLOBALS['devel_shutdown'] = FALSE;
  print _wysiwyg_mathjax_to_js(array('status' => TRUE, 'data' => array('iid' => $state['values']['iid'])));
  exit();
}

/**
 * Return the form to be presented as image details form.
 * Title, floating description and so forth
 */
function _wysiwyg_mathjax_details_form($formula, $iid, $defaults = array()) {
  $form = array();

  $form['formula_upload_details'] = array(
    '#type' => 'fieldset',
    '#title' => t('Formula details'),
    '#prefix' => '<div id="formula_details">',
    '#suffix' => '</div>'
  );
  // Title
  $form['formula_upload_details']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $defaults['title'],
    '#size' => 50
  );
  // Alignment / Floating
  $m = drupal_get_path('module','wysiwyg_mathjax');
  $form['formula_upload_details']['alignment'] = array(
    '#type' => 'select',
    '#title' => t('Formula alignment'),
    '#options' => array(
//TODO Not working yet      'mathjax_inline' => theme('image',"$m/images/inline.png", $t = t('Formula flows inline with the text'), $t),
      'mathjax_floating_left' => theme('image',"$m/images/left_float.png", $t = t('Formula on left, text on right'), $t),
      'mathjax_floating_none_left' => theme('image',"$m/images/none_left_float.png", $t = t('Formula is inline with the text (left)'), $t),
      'mathjax_floating_none' => theme('image',"$m/images/none_float.png", $t = t('Formula is inline with the text (none)'), $t),
      'mathjax_floating_none_right' => theme('image',"$m/images/none_right_float.png", $t = t('Formula is inline with the text (right)'), $t),
      'mathjax_floating_right' => theme('image',"$m/images/right_float.png", $t = t('Image on right, text on left'), $t),      
    ),
    '#default_value' => $defaults['alignment'],
    '#after_build' => array('drupalwiki_imageselect_element_register'),
  );

  // We group all those extras into this fieldset to hide them from the general UI.
  $form['formula_upload_details']['extras'] = array(
    '#type' => 'fieldset',
    '#title' => t('Extras'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );

  // image styles.
  $styles = _wysiwyg_mathjax_get_formula_styles();
  if (count($styles) > 0) {
    array_unshift($styles, t('None'));
    $form['formula_upload_details']['extras']['style'] = array(
      '#type' => 'select',
      '#title' => t('Style'),
      '#options' => $styles,
      '#default_value' => $defaults['style'],
    );
  }

  // this is the container for our preview-image
  $form['formula_upload_details']['preview'] = array(
    '#type' => 'fieldset',
    '#title' => t('Preview'),
    '#attributes' => array('id' => 'formula_preview'),
    '#collapsible' => false
  );
  $form['formula_upload_details']['preview'] ['uploadedFormula'] = array(
    '#prefix' => '<div >',
    '#suffix' => '</div>',
    '#value' => $formula,
  );

  // This is a fake submit button, thats why "access false" is used.
  // This is because we use an ajax form submit.
  $form['formula_upload_details']['buttons']['submitformuladetails'] = array(
    '#type' => 'submit',
    '#value' => 'JSinsert', // t() not needed here, see comment below.
    '#access' => FALSE,
  );

  $form['formula_upload_details']['iid'] = array(
    '#type' => 'value',
    '#value' => $iid,
    '#default_value' => $iid
  );
  return $form;
}
