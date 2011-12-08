open Str

let rec take_while p lst = match lst with 
  | [] -> []
  | x::xs -> if p x then x :: (take_while p xs) else []

let rec intersperse sep lst = match lst with
  | [] -> []
  | x::[] -> [x]
  | x::xs -> x :: sep :: (intersperse sep xs)

let (--) i j = 
  let rec aux n acc =
    if n < i then acc else aux (n-1) (n :: acc)
  in aux j []

let rec zip l1 l2 = match l1, l2 with
    x::xs, y::ys -> (x, y) :: zip xs ys
  | [], [] -> []
  | _, _ -> []

let map f l =
  let rec inner l accum = match l with
    | x::xs -> inner xs ((f x) :: accum)
    | []    -> List.rev accum
  in inner l [] ;;

let last l = List.hd (List.rev l)

let rec replace_one_by fn a b l = match l with
    x::xs ->
      if fn a x
      then b :: xs
      else x :: replace_one_by fn a b xs
  | [] -> []

let split_str sep str =
  Str.split (Str.regexp_string sep) str

let error msg = Format.printf "@[error: %s@]@." msg

let (%) = Printf.sprintf

let read_file_as_string path =
  (* use open_in_bin to avoid line ending issues on Windows *)
  let chan = open_in_bin path in
  let len = in_channel_length chan in
  let str = String.create len in
  really_input chan str 0 len;
  close_in chan;
  (str)

let base_library_directory =
  try
    Sys.getenv "PRIMER_LIBRARY_PATH"
  with
    | Sys_error _ -> error "PRIMER_LIBRARY_PATH not set"; exit 0

let base_library = Filename.concat base_library_directory "lib.pri"

let library_path file =
  Filename.concat base_library_directory file ^ ".pri"
